#include "common.h"

#ifdef USE_RTX
#include "cmsis_os.h"
#endif

volatile uint32_t msTicks;
static uint32_t last_keys;
static uint32_t seed = 1;

#ifdef USE_RTX
/*
 * RTX version: three threads plus main.
 *   input_thread  (above normal): samples the joystick every 10 ms and queues each new press,
 *                                 so presses made while an app is busy drawing are never lost
 *   main          (normal):       the menu and the apps (gallery, audio, Tetris, Snake)
 *   clock_thread  (below normal): redraws the uptime clock on the menu once a second
 * The LCD is shared by main and clock_thread, so every draw goes through lcd_mutex.
 * RTX owns SysTick for its scheduler, so the 1 ms game clock (msTicks) runs on TIMER1.
 */
void game_clock_init(void);              /* game_clock.c: TIMER1 -> msTicks++ every 1 ms */

osMessageQDef(key_queue, 16, uint32_t);
static osMessageQId key_queue;
osMutexDef(lcd_mutex);
static osMutexId lcd_mutex;
volatile int menu_visible;               /* clock_thread only draws while the menu is on screen */

void lcd_lock(void)   { osMutexWait(lcd_mutex, osWaitForever); }
void lcd_unlock(void) { osMutexRelease(lcd_mutex); }

/* Other threads run while this one waits, instead of burning the CPU in a busy loop. */
void delay_ms(uint32_t ms) { osDelay(ms); }

static void input_thread(void const *arg) {
  uint32_t now;
  (void)arg;
  for (;;) {
    now = get_button();
    if (now & ~last_keys) osMessagePut(key_queue, now & ~last_keys, 0);
    last_keys = now;
    osDelay(10);
  }
}
osThreadDef(input_thread, osPriorityAboveNormal, 1, 0);

void clock_update(void) {
  char buf[8];
  uint32_t s = msTicks / 1000;
  lcd_lock();
  if (menu_visible) {
    sprintf(buf, "%02u:%02u", (unsigned)((s / 60) % 100), (unsigned)(s % 60));
    text(1, 46, 0, LightGrey, Navy, buf);
  }
  lcd_unlock();
}

static void clock_thread(void const *arg) {
  (void)arg;
  for (;;) {
    clock_update();
    osDelay(1000);
  }
}
osThreadDef(clock_thread, osPriorityBelowNormal, 1, 0);

uint32_t joy_held(void) { return get_button(); }

/* All presses queued since the last call, combined. */
uint32_t joy_pressed(void) {
  uint32_t keys = 0;
  osEvent e;
  for (;;) {
    e = osMessageGet(key_queue, 0);
    if (e.status != osEventMessage) break;
    keys |= e.value.v;
  }
  return keys;
}

void joy_wait_release(void) {
  while (get_button()) delay_ms(10);
  delay_ms(30);
  joy_pressed();                         /* drop presses left over from the release */
}

void rtx_start(void) {
  osKernelInitialize();
  game_clock_init();
  key_queue = osMessageCreate(osMessageQ(key_queue), NULL);
  lcd_mutex = osMutexCreate(osMutex(lcd_mutex));
  osThreadCreate(osThread(input_thread), NULL);
  osThreadCreate(osThread(clock_thread), NULL);
  osKernelStart();                       /* main keeps running as the UI thread */
}

#else
/* Bare-metal version: one loop, SysTick drives the 1 ms clock. */
void lcd_lock(void)   { }
void lcd_unlock(void) { }

void SysTick_Handler(void) { msTicks++; }

void delay_ms(uint32_t ms) {
  uint32_t start = msTicks;
  while ((msTicks - start) < ms);
}

uint32_t joy_held(void) { return get_button(); }

uint32_t joy_pressed(void) {
  uint32_t now = get_button();
  uint32_t p = now & ~last_keys;
  last_keys = now;
  return p;
}

void joy_wait_release(void) {
  while (get_button()) delay_ms(10);
  delay_ms(30);
  last_keys = 0;
}
#endif

void rnd_seed(uint32_t s) { seed = s ? s : 1; }

uint32_t rnd(void) {
  seed = seed * 1103515245u + 12345u;
  return seed >> 16;
}

void fill_rect(int x, int y, int w, int h, uint16_t c) {
  lcd_lock();
  GLCD_SetTextColor(c);
  GLCD_Bargraph(x, y, w, h, 1024);
  lcd_unlock();
}

void text(int ln, int col, int big, uint16_t fg, uint16_t bg, const char *s) {
  lcd_lock();
  GLCD_SetTextColor(fg);
  GLCD_SetBackColor(bg);
  GLCD_DisplayString(ln, col, big, (unsigned char *)s);
  lcd_unlock();
}

static const char *items[] = {
  " Photo Gallery    ",
  " MP3 Player       ",
  " Tetris           ",
  " Snake            "
};
#define N_ITEMS 4

static void draw_item(int i, int on) {
  if (on) text(3 + i, 1, 1, Black, Yellow, items[i]);
  else    text(3 + i, 1, 1, White, Navy, items[i]);
}

static void draw_menu(int sel) {
  int i;
  lcd_lock();
  GLCD_Clear(Navy);
  text(1, 4, 1, Yellow, Navy, "MEDIA CENTER");
  for (i = 0; i < N_ITEMS; i++) draw_item(i, i == sel);
  text(9, 1, 1, LightGrey, Navy, "UP/DOWN  SELECT=go");
  LED_Out(1 << sel);
#ifdef USE_RTX
  menu_visible = 1;
#endif
  lcd_unlock();
#ifdef USE_RTX
  clock_update();
#endif
}

extern uint32_t SystemFrequency;
extern void SystemClockUpdate(void);
#ifdef PROFILE
void profile_run(void);
#endif

int main(void) {
  int sel = 0;
  uint32_t k;

  SystemClockUpdate();
#ifndef USE_RTX
  SysTick_Config(SystemFrequency / 1000);
#endif
  LED_Init();
  KBD_Init();
  GLCD_Init();
#ifdef PROFILE
  profile_run();
#endif

#ifdef USE_RTX
  rtx_start();
#endif
  draw_menu(sel);
  while (1) {
    k = joy_pressed();
    if (k & KBD_UP) {
      draw_item(sel, 0);
      sel = (sel + N_ITEMS - 1) % N_ITEMS;
      draw_item(sel, 1);
      LED_Out(1 << sel);
    }
    if (k & KBD_DOWN) {
      draw_item(sel, 0);
      sel = (sel + 1) % N_ITEMS;
      draw_item(sel, 1);
      LED_Out(1 << sel);
    }
    if (k & KBD_SELECT) {
#ifdef USE_RTX
      lcd_lock();
      menu_visible = 0;                  /* the app owns the screen now */
      lcd_unlock();
#endif
      joy_wait_release();
      switch (sel) {
        case 0: gallery_run(); break;
        case 1: audio_run();   break;
        case 2: tetris_run();  break;
        case 3: snake_run();   break;
      }
      joy_wait_release();
      draw_menu(sel);
    }
    delay_ms(20);
  }
}
