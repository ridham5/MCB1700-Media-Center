#include "check.h"
#include "common.h"
#include "fake_hw.h"

static uint32_t script_value;
static uint32_t script(void) { return script_value; }

TEST(press_counts_once_while_held) {
  fake_buttons = script;
  script_value = 0;
  joy_pressed();
  script_value = KBD_UP;
  CHECK_EQ(joy_pressed(), KBD_UP);
  CHECK_EQ(joy_pressed(), 0);
  CHECK_EQ(joy_pressed(), 0);
  CHECK_EQ(joy_held(), KBD_UP);
}

TEST(second_button_while_first_held) {
  fake_buttons = script;
  script_value = KBD_UP;
  joy_pressed();
  script_value = KBD_UP | KBD_SELECT;
  CHECK_EQ(joy_pressed(), KBD_SELECT);
}

TEST(release_then_press_again) {
  fake_buttons = script;
  script_value = KBD_LEFT;
  joy_pressed();
  script_value = 0;
  CHECK_EQ(joy_pressed(), 0);
  script_value = KBD_LEFT;
  CHECK_EQ(joy_pressed(), KBD_LEFT);
}

TEST(random_is_repeatable_for_a_seed) {
  uint32_t a[5], i;
  rnd_seed(1234);
  for (i = 0; i < 5; i++) a[i] = rnd();
  rnd_seed(1234);
  for (i = 0; i < 5; i++) CHECK_EQ(rnd(), a[i]);
  rnd_seed(0);
  CHECK(rnd() != 0 || rnd() != 0);
}

TEST(fill_rect_paints_exact_area) {
  GLCD_Clear(Black);
  fill_rect(10, 20, 5, 3, Red);
  CHECK_EQ(fb[20][10], Red);
  CHECK_EQ(fb[22][14], Red);
  CHECK_EQ(fb[23][14], Black);
  CHECK_EQ(fb[20][15], Black);
  CHECK_EQ(fb[19][10], Black);
}

TEST(delay_waits_for_the_tick) {
  uint32_t start;
  fake_clock_start(20);
  start = fake_now();
  delay_ms(50);
  CHECK(fake_now() - start >= 50);
  fake_clock_stop();
}

int main(void) {
  RUN(press_counts_once_while_held);
  RUN(second_button_while_first_held);
  RUN(release_then_press_again);
  RUN(random_is_repeatable_for_a_seed);
  RUN(fill_rect_paints_exact_area);
  RUN(delay_waits_for_the_tick);
  DONE();
}
