#define _DEFAULT_SOURCE
#include <unistd.h>
#include <pthread.h>
#include "check.h"
#include "common.h"
#include "fake_hw.h"

/* Runs the RTX build (USE_RTX) on pthreads: real threads, signals, queue and mutex. */

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

/* ---- game thread ---- */
static pthread_t game_ran_on;
static volatile int game_runs;
static void fake_game(void) { game_ran_on = pthread_self(); delay_ms(30); game_runs++; }

TEST(game_runs_on_its_own_thread_and_ui_waits) {
  game_runs = 0;
  run_game(fake_game);                 /* returns only after the game is done */
  CHECK_EQ(game_runs, 1);
  CHECK(!pthread_equal(game_ran_on, pthread_self()));
  run_game(fake_game);                 /* and it can be started again */
  CHECK_EQ(game_runs, 2);
}

/* ---- audio + volume threads ---- */
extern int fake_usb_connected;
extern uint16_t PotVal;
extern volatile int audio_playing;

static int bar_green_pixels(void) {
  int x, n = 0;
  for (x = VOL_BAR_X; x < VOL_BAR_X + VOL_BAR_W; x++)
    if (fb[VOL_BAR_Y + 1][x] == Green) n++;
  return n;
}

TEST(audio_thread_starts_and_stops_the_stream) {
  fake_usb_connected = 0;
  rtx_audio(1);
  CHECK_EQ(fake_usb_connected, 1);
  CHECK_EQ(audio_playing, 1);
  rtx_audio(0);
  CHECK_EQ(fake_usb_connected, 0);
  CHECK_EQ(audio_playing, 0);
  CHECK_EQ(LPC_DAC->CR, 0x00008000);   /* DAC parked at mid-scale: silence */
}

TEST(volume_bar_follows_the_knob_while_playing) {
  GLCD_Clear(Black);
  PotVal = 255;
  rtx_audio(1);
  delay_ms(250);
  CHECK_EQ(bar_green_pixels(), VOL_BAR_W);         /* full knob = full bar */
  PotVal = 0;
  delay_ms(250);
  CHECK_EQ(bar_green_pixels(), 0);                 /* knob down = empty bar */
  PotVal = 128;
  delay_ms(250);
  CHECK_EQ(bar_green_pixels(), 128 * VOL_BAR_W / 255);
  rtx_audio(0);
  GLCD_Clear(Black);
  PotVal = 255;
  delay_ms(250);
  CHECK_EQ(bar_green_pixels(), 0);                 /* no music, no bar */
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
  RUN(game_runs_on_its_own_thread_and_ui_waits);
  RUN(audio_thread_starts_and_stops_the_stream);
  RUN(volume_bar_follows_the_knob_while_playing);
  fake_clock_stop();
  DONE();
}
