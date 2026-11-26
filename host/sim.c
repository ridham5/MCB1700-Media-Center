#include <stdlib.h>
#include <string.h>
#include "fake_hw.h"

#if SIM_APP == 1
#include "common.h"
int firmware_main(void);
static uint32_t t0;
static uint32_t script(void) {
  uint32_t t = fake_now() - t0;
  static const uint32_t at[] = { 600, 1200, 1800, 2400, 3000, 3600 };
  static const uint32_t key[] = { KBD_DOWN, KBD_DOWN, KBD_DOWN, KBD_UP, KBD_UP, KBD_UP };
  unsigned i;
  for (i = 0; i < 6; i++) if (t >= at[i] && t < at[i] + 80) return key[i];
  return 0;
}
static void run(void) { t0 = fake_now(); fake_buttons = script; fake_stop_at = t0 + 4200; firmware_main(); }
#elif SIM_APP == 2
#include "../src/gallery.c"
static uint32_t t0;
static uint32_t script(void) {
  uint32_t t = fake_now() - t0, s = t / 1500;
  return (t % 1500 < 80 && s >= 1 && s <= 3) ? KBD_RIGHT : 0;
}
static void run(void) { t0 = fake_now(); fake_buttons = script; fake_stop_at = t0 + 6000; gallery_run(); }
#elif SIM_APP == 3
#include "../src/tetris.c"
#include "tetris_bot.h"
static void run(void) { fake_buttons = tetris_bot; fake_stop_at = fake_now() + 16000; tetris_run(); }
#elif SIM_APP == 4
#include "../src/snake.c"
#include "snake_bot.h"
static void run(void) { fake_buttons = snake_bot; fake_stop_at = fake_now() + 12000; snake_run(); }
#elif SIM_APP == 5
#include "../src/audio.c"
static void run(void) { fake_stop_at = fake_now() + 2500; audio_run(); }
#endif

int main(int argc, char **argv) {
  GLCD_Init();
  fake_clock_start(200);
  fake_capture(argc > 1 ? argv[1] : "frames.raw", argc > 2 ? (uint32_t)atoi(argv[2]) : 100);
  run();
  fake_capture_flush();
  return 0;
}
