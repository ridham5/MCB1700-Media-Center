#include "check.h"
#include "fake_hw.h"
#include "../src/gallery.c"

static void check_photo_on_screen(int i) {
  const Photo *p = &photos[i];
  const unsigned short *px = (const unsigned short *)p->data;
  int x0 = (320 - p->w) / 2, y0 = 36, r, c, bad = 0;
  for (r = 0; r < p->h; r++)
    for (c = 0; c < p->w; c++)
      if (fb[y0 + r][x0 + c] != px[r * p->w + c]) bad++;
  CHECK_EQ(bad, 0);
}

TEST(each_photo_is_drawn_upright_and_unmirrored) {
  int i;
  for (i = 0; i < (int)N_PHOTOS; i++) {
    GLCD_Clear(Black);
    show(i);
    check_photo_on_screen(i);
    CHECK_EQ(fake_leds, 1u << i);
  }
}

TEST(photos_are_160_by_120) {
  int i;
  for (i = 0; i < (int)N_PHOTOS; i++) {
    CHECK_EQ(photos[i].w, 160);
    CHECK_EQ(photos[i].h, 120);
  }
}

static int calls;
static uint32_t right_twice_then_left_three_times(void) {
  static const uint32_t seq[] = { KBD_RIGHT, KBD_RIGHT, KBD_LEFT, KBD_LEFT, KBD_LEFT };
  int n = calls++;
  if (n >= 60) return KBD_SELECT;
  if (n % 10 == 5 && n / 10 < 5) return seq[n / 10];
  return 0;
}

TEST(right_and_left_step_through_and_wrap_around) {
  calls = 0;
  fake_buttons = right_twice_then_left_three_times;
  fake_clock_start(10);
  gallery_run();
  fake_clock_stop();
  check_photo_on_screen(3);
}

int main(void) {
  RUN(each_photo_is_drawn_upright_and_unmirrored);
  RUN(photos_are_160_by_120);
  RUN(right_and_left_step_through_and_wrap_around);
  DONE();
}
