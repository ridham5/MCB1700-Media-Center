#include <stdlib.h>
#include "check.h"
#include "fake_hw.h"
#include "../src/snake.c"
#include "snake_bot.h"

static uint32_t t0;
static uint32_t no_input_then_quit(void) {
  return (fake_now() - t0 > 5000 && ((fake_now() / 40) & 1)) ? KBD_SELECT : 0;
}

static uint32_t reverse_then_quit(void) {
  if (fake_now() - t0 < 300) return KBD_LEFT;
  return no_input_then_quit();
}

TEST(snake_without_input_hits_the_right_wall) {
  fake_buttons = no_input_then_quit;
  fake_clock_start(10);
  t0 = fake_now();
  snake_run();
  fake_clock_stop();
  CHECK_EQ(sx[0], COLS - 1);
  CHECK_EQ(sy[0], 7);
  CHECK_EQ(len, 3);
}

TEST(snake_cannot_reverse_into_itself) {
  fake_buttons = reverse_then_quit;
  fake_clock_start(10);
  t0 = fake_now();
  snake_run();
  fake_clock_stop();
  CHECK_EQ(sx[0], COLS - 1);
}

TEST(food_never_spawns_on_the_snake) {
  int i, k;
  len = 6;
  for (i = 0; i < len; i++) { sx[i] = i; sy[i] = 0; }
  GLCD_Clear(Black);
  for (k = 0; k < 500; k++) {
    place_food();
    CHECK(!on_snake(fx, fy, len));
  }
}

TEST(snake_grows_by_one_per_apple_and_scores_10) {
  snake_quit_at = 0;
  fake_buttons = snake_bot;
  fake_clock_start(10);
  snake_quit_at = fake_now() + 30000;
  snake_run();
  fake_clock_stop();
  printf("  bot reached length %d, score %lu\n", len, (unsigned long)score);
  CHECK(score >= 30);
  CHECK_EQ(len, 3 + (int)(score / 10));
}

int main(void) {
  RUN(snake_without_input_hits_the_right_wall);
  RUN(snake_cannot_reverse_into_itself);
  RUN(food_never_spawns_on_the_snake);
  RUN(snake_grows_by_one_per_apple_and_scores_10);
  DONE();
}
