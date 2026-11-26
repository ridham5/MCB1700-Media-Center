#include <string.h>
#include "check.h"
#include "fake_hw.h"
#define HIGHSCORES
#include "../src/highscore.c"
#include "../src/tetris.c"

static uint32_t flash[64];
static int writes;

const void *hs_flash_addr(void) { return flash; }
int hs_flash_write(const void *src, uint32_t len) {
  writes++;
  memcpy(flash, src, len);
  return 0;
}

static void reset_flash(void) {
  memset(flash, 0xFF, sizeof flash);
  loaded = 0;
  writes = 0;
}

TEST(blank_flash_reads_as_zero) {
  reset_flash();
  CHECK_EQ(hs_get(HS_TETRIS), 0);
  CHECK_EQ(hs_get(HS_SNAKE), 0);
  CHECK_EQ(writes, 0);
}

TEST(higher_score_is_saved_lower_is_not) {
  reset_flash();
  CHECK_EQ(hs_submit(HS_SNAKE, 120), 1);
  CHECK_EQ(writes, 1);
  CHECK_EQ(hs_submit(HS_SNAKE, 80), 0);
  CHECK_EQ(hs_submit(HS_SNAKE, 120), 0);
  CHECK_EQ(writes, 1);
  CHECK_EQ(hs_get(HS_SNAKE), 120);
  CHECK_EQ(hs_get(HS_TETRIS), 0);
}

TEST(scores_survive_a_reboot) {
  reset_flash();
  hs_submit(HS_TETRIS, 560);
  hs_submit(HS_SNAKE, 90);
  loaded = 0;
  memset(&cache, 0, sizeof cache);
  CHECK_EQ(hs_get(HS_TETRIS), 560);
  CHECK_EQ(hs_get(HS_SNAKE), 90);
}

TEST(block_is_one_256_byte_flash_page) {
  CHECK_EQ(sizeof(HsBlock), 256);
}

static uint32_t drop_start;
static uint32_t drop_until_game_over(void) {
  uint32_t t = fake_now() - drop_start;
  if (t > 60000) return ((t / 40) & 1) ? KBD_SELECT : 0;
  return KBD_DOWN;
}

TEST(finished_tetris_game_records_its_score) {
  int c, top = 0;
  reset_flash();
  fake_buttons = drop_until_game_over;
  fake_clock_start(10);
  drop_start = fake_now();
  tetris_run();
  for (c = 0; c < BW; c++) top |= board[0][c] | board[1][c] | board[2][c] | board[3][c];
  CHECK(top);
  fake_clock_stop();
  CHECK_EQ(hs_get(HS_TETRIS), score);
  CHECK_EQ(writes, score > 0 ? 1 : 0);
}

int main(void) {
  RUN(blank_flash_reads_as_zero);
  RUN(higher_score_is_saved_lower_is_not);
  RUN(scores_survive_a_reboot);
  RUN(block_is_one_256_byte_flash_page);
  RUN(finished_tetris_game_records_its_score);
  DONE();
}
