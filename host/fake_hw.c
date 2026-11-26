#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "fake_hw.h"
#include "GLCD.h"
#include "KBD.h"
#include "LED.h"
#include "LPC17xx.h"

#ifdef KEIL_FONTS
#include "Font_6x8_h.h"
#include "Font_16x24_h.h"
#endif

extern volatile uint32_t msTicks;

uint16_t fb[LCD_H][LCD_W];
unsigned int fake_leds;
uint32_t (*fake_buttons)(void);
uint32_t fake_stop_at;
uint32_t SystemFrequency = 100000000;

static uint16_t text_color = White, back_color = Black;

uint32_t SysTick_Config(uint32_t ticks) { (void)ticks; return 0; }

static FILE *cap_file;
static uint32_t cap_every, cap_next;
static uint16_t (*cap_buf)[LCD_H][LCD_W];
static size_t cap_n, cap_max;

static void on_tick(int sig) {
  (void)sig;
  msTicks++;
  if (cap_file && msTicks >= cap_next && cap_n < cap_max) {
    memcpy(cap_buf[cap_n++], fb, sizeof fb);
    cap_next = msTicks + cap_every;
  }
  if (fake_stop_at && msTicks >= fake_stop_at) {
    fake_capture_flush();
    _exit(0);
  }
}

void fake_clock_start(unsigned int us_per_ms) {
  struct itimerval it;
  struct sigaction sa;
  memset(&sa, 0, sizeof sa);
  sa.sa_handler = on_tick;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGALRM, &sa, NULL);
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = us_per_ms;
  it.it_value = it.it_interval;
  setitimer(ITIMER_REAL, &it, NULL);
}

void fake_clock_stop(void) {
  struct itimerval it;
  memset(&it, 0, sizeof it);
  setitimer(ITIMER_REAL, &it, NULL);
}

uint32_t fake_now(void) { return msTicks; }

void fake_capture(const char *path, uint32_t every_ms) {
  cap_file = fopen(path, "wb");
  cap_every = every_ms;
  cap_next = msTicks;
  cap_max = 2000;
  cap_buf = malloc(cap_max * sizeof *cap_buf);
}

void fake_capture_flush(void) {
  if (!cap_file) return;
  fwrite(cap_buf, sizeof *cap_buf, cap_n, cap_file);
  fclose(cap_file);
  cap_file = NULL;
}

void GLCD_Init(void) { memset(fb, 0, sizeof fb); }
void GLCD_SetTextColor(unsigned short c) { text_color = c; }
void GLCD_SetBackColor(unsigned short c) { back_color = c; }

void GLCD_Clear(unsigned short c) {
  int x, y;
  for (y = 0; y < LCD_H; y++) for (x = 0; x < LCD_W; x++) fb[y][x] = c;
}

static void put(int x, int y, uint16_t c) {
  if (x >= 0 && x < LCD_W && y >= 0 && y < LCD_H) fb[y][x] = c;
}

void GLCD_Bargraph(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int val) {
  unsigned int i, j;
  val = (val * w) >> 10;
  for (i = 0; i < h; i++)
    for (j = 0; j < w; j++)
      put(x + j, y + i, j >= val ? back_color : text_color);
}

void GLCD_Bitmap(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap) {
  const unsigned short *p = (const unsigned short *)bitmap;
  unsigned int r, c;
  for (r = 0; r < h; r++)
    for (c = 0; c < w; c++)
      put(x + c, y + r, p[(h - 1 - r) * w + c]);
}

static void draw_char(int x, int y, int cw, int ch, int big, unsigned char c) {
  int i, j;
  for (j = 0; j < ch; j++)
    for (i = 0; i < cw; i++) {
      int on = 0;
#ifdef KEIL_FONTS
      if (c >= 32) {
        unsigned int bits = big ? Font_16x24_h[(c - 32) * 24 + j] : Font_6x8_h[(c - 32) * 8 + j];
        on = (bits >> i) & 1;
      }
#else
      (void)big;
      on = c > 32 && i > 0 && i < cw - 1 && j > ch / 4 && j < ch - 2;
#endif
      put(x + i, y + j, on ? text_color : back_color);
    }
}

void GLCD_DisplayString(unsigned int ln, unsigned int col, unsigned char fi, unsigned char *s) {
  while (*s) {
    if (fi) draw_char(col * 16, ln * 24, 16, 24, 1, *s);
    else    draw_char(col * 6, ln * 8, 6, 8, 0, *s);
    col++; s++;
  }
}

void KBD_Init(void) {}
uint32_t get_button(void) { return fake_buttons ? fake_buttons() : 0; }

void LED_Init(void) {}
void LED_On(unsigned int n) { fake_leds |= 1u << n; }
void LED_Off(unsigned int n) { fake_leds &= ~(1u << n); }
void LED_Out(unsigned int v) { fake_leds = v; }

void SystemClockUpdate(void) {}

HOST_TIM_TypeDef host_tim0;
HOST_DAC_TypeDef host_dac;
int fake_usb_connected;
void NVIC_DisableIRQ(IRQn_Type irq) { (void)irq; }
void audio_start(void) { fake_usb_connected = 1; host_tim0.TCR = 1; }
void USB_Connect(uint32_t con) { fake_usb_connected = (int)con; }

__attribute__((weak)) void gallery_run(void) {}
__attribute__((weak)) void audio_run(void) {}
__attribute__((weak)) void tetris_run(void) {}
__attribute__((weak)) void snake_run(void) {}
