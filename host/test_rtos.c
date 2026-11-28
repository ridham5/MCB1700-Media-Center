#define _DEFAULT_SOURCE
#include <unistd.h>
#include "check.h"
#include "common.h"
#include "fake_hw.h"

/* Runs the RTX build (USE_RTX) on pthreads: real threads, queue and mutex. */

void rtx_start(void);
void clock_update(void);
extern volatile int menu_visible;

static volatile uint32_t buttons;
static uint32_t script(void) { return buttons; }

static void hold(uint32_t keys, uint32_t ms) { buttons = keys; delay_ms(ms); }

TEST(press_while_app_is_busy_is_not_lost) {
  joy_pressed();                       /* empty the queue */
  hold(KBD_UP, 40);                    /* user taps UP while the app is busy... */
  hold(0, 40);
  CHECK_EQ(joy_pressed(), KBD_UP);     /* ...and the input thread kept it for us */
  CHECK_EQ(joy_pressed(), 0);
}

TEST(holding_a_key_counts_as_one_press) {
  joy_pressed();
  hold(KBD_LEFT, 200);
  CHECK_EQ(joy_pressed(), KBD_LEFT);
  hold(0, 30);
  CHECK_EQ(joy_pressed(), 0);
}

TEST(two_taps_are_both_seen) {
  joy_pressed();
  hold(KBD_UP, 30);
  hold(0, 30);
  hold(KBD_DOWN, 30);
  hold(0, 30);
  CHECK_EQ(joy_pressed(), KBD_UP | KBD_DOWN);
}

TEST(delay_lets_other_threads_run) {
  uint32_t start = fake_now();
  delay_ms(50);
  CHECK(fake_now() - start >= 50);
}

static int clock_area_changed(uint16_t before[8][30]) {
  int x, y;
  for (y = 0; y < 8; y++) for (x = 0; x < 30; x++)
    if (fb[8 + y][276 + x] != before[y][x]) return 1;
  return 0;
}

static void snapshot(uint16_t out[8][30]) {
  int x, y;
  for (y = 0; y < 8; y++) for (x = 0; x < 30; x++) out[y][x] = fb[8 + y][276 + x];
}

TEST(clock_draws_only_while_menu_is_showing) {
  uint16_t before[8][30];
  GLCD_Clear(Black);
  menu_visible = 0;                    /* an app owns the screen */
  snapshot(before);
  clock_update();
  CHECK(!clock_area_changed(before));
  menu_visible = 1;                    /* back on the menu */
  clock_update();
  CHECK(clock_area_changed(before));
  menu_visible = 0;
}

int main(void) {
  fake_buttons = script;
  fake_clock_start(200);               /* 1 board-ms = 0.2 real ms */
  rtx_start();
  usleep(2000);
  RUN(press_while_app_is_busy_is_not_lost);
  RUN(holding_a_key_counts_as_one_press);
  RUN(two_taps_are_both_seen);
  RUN(delay_lets_other_threads_run);
  RUN(clock_draws_only_while_menu_is_showing);
  fake_clock_stop();
  DONE();
}
