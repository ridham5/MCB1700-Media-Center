#include "common.h"

volatile uint32_t msTicks;
static uint32_t last_keys;
static uint32_t seed = 1;

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

void rnd_seed(uint32_t s) { seed = s ? s : 1; }

uint32_t rnd(void) {
  seed = seed * 1103515245u + 12345u;
  return seed >> 16;
}

void fill_rect(int x, int y, int w, int h, uint16_t c) {
  GLCD_SetTextColor(c);
  GLCD_Bargraph(x, y, w, h, 1024);
}

void text(int ln, int col, int big, uint16_t fg, uint16_t bg, const char *s) {
  GLCD_SetTextColor(fg);
  GLCD_SetBackColor(bg);
  GLCD_DisplayString(ln, col, big, (unsigned char *)s);
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
  GLCD_Clear(Navy);
  text(1, 4, 1, Yellow, Navy, "MEDIA CENTER");
  for (i = 0; i < N_ITEMS; i++) draw_item(i, i == sel);
  text(9, 1, 1, LightGrey, Navy, "UP/DOWN  SELECT=go");
  LED_Out(1 << sel);
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
  SysTick_Config(SystemFrequency / 1000);
  LED_Init();
  KBD_Init();
  GLCD_Init();
#ifdef PROFILE
  profile_run();
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
