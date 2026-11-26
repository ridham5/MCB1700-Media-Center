#ifdef PROFILE
#include "common.h"

#define DEMCR      (*(volatile uint32_t *)0xE000EDFCu)
#define DWT_CTRL   (*(volatile uint32_t *)0xE0001000u)
#define DWT_CYCCNT (*(volatile uint32_t *)0xE0001004u)

extern uint32_t SystemFrequency;

static uint32_t t_start;

static void start(void) { t_start = DWT_CYCCNT; }
static uint32_t stop_us(void) { return (DWT_CYCCNT - t_start) / (SystemFrequency / 1000000u); }

static void row(int ln, const char *name, uint32_t us) {
  char buf[40];
  sprintf(buf, "%-22s %7lu us", name, (unsigned long)us);
  text(ln, 1, 0, White, Black, buf);
}

void profile_run(void) {
  static unsigned short line[160];
  uint32_t r[6];
  int i;

  DEMCR |= 1u << 24;
  DWT_CYCCNT = 0;
  DWT_CTRL |= 1u;

  start(); GLCD_Clear(Black); r[0] = stop_us();
  start(); fill_rect(0, 0, 11, 11, Red); r[1] = stop_us();
  start(); for (i = 0; i < 200; i++) fill_rect(20 + (i % 10) * 11, 10 + (i / 10) * 11, 10, 10, Blue); r[2] = stop_us();
  start(); text(5, 0, 1, White, Black, "ABCDEFGHIJKLMNOPQRST"); r[3] = stop_us();
  for (i = 0; i < 160; i++) line[i] = (unsigned short)i;
  start(); for (i = 0; i < 120; i++) GLCD_Bitmap(80, 36 + i, 160, 1, (unsigned char *)line); r[4] = stop_us();
  start(); delay_ms(10); r[5] = stop_us();

  GLCD_Clear(Black);
  text(0, 1, 1, Yellow, Black, "TIMING (DWT)");
  row(5, "Full screen clear", r[0]);
  row(6, "One 11x11 cell", r[1]);
  row(7, "Full Tetris board", r[2]);
  row(8, "One line of big text", r[3]);
  row(9, "One 160x120 photo", r[4]);
  row(10, "delay_ms(10) check", r[5]);
  text(27, 1, 0, LightGrey, Black, "SELECT = continue");
  while (!(joy_pressed() & KBD_SELECT)) delay_ms(20);
  joy_wait_release();
}
#endif
