#include <stdlib.h>
#include <string.h>
#include "check.h"
#include "fake_hw.h"
#include "../src/tetris.c"
#include "tetris_bot.h"

static void clear_board(void) {
  memset(board, 0, sizeof board);
  score = lines = level = 0;
}

TEST(every_shape_has_four_cells_in_every_rotation) {
  int t, r, i, j, n;
  for (t = 0; t < 7; t++)
    for (r = 0; r < 4; r++) {
      n = 0;
      for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) n += cell_on(t, r, i, j);
      CHECK_EQ(n, 4);
    }
}

TEST(o_piece_looks_the_same_after_rotating) {
  CHECK_EQ(shapes[3][0], shapes[3][1]);
  CHECK_EQ(shapes[3][1], shapes[3][2]);
}

TEST(walls_and_floor_block_moves) {
  clear_board();
  CHECK(fits(0, 0, 0, 0));
  CHECK(fits(0, 0, 6, 0));
  CHECK(!fits(0, 0, 7, 0));
  CHECK(!fits(0, 0, -1, 0));
  CHECK(fits(0, 0, 0, 18));
  CHECK(!fits(0, 0, 0, 19));
}

TEST(stacked_blocks_block_moves) {
  clear_board();
  board[10][4] = Red;
  CHECK(!fits(3, 0, 3, 9));
  CHECK(fits(3, 0, 3, 8));
  CHECK(fits(3, 0, 5, 9));
}

TEST(piece_can_hang_above_the_top) {
  clear_board();
  CHECK(fits(0, 1, 0, -1));
}

TEST(one_full_row_clears_and_scores_40) {
  int c;
  clear_board();
  GLCD_Clear(Black);
  for (c = 0; c < BW; c++) board[19][c] = Blue;
  board[18][2] = Red;
  clear_lines();
  CHECK_EQ(lines, 1);
  CHECK_EQ(score, 40);
  CHECK_EQ(board[19][2], Red);
  CHECK_EQ(board[18][2], 0);
}

TEST(four_rows_at_once_score_1200) {
  int r, c;
  clear_board();
  GLCD_Clear(Black);
  for (r = 16; r < 20; r++) for (c = 0; c < BW; c++) board[r][c] = Green;
  clear_lines();
  CHECK_EQ(lines, 4);
  CHECK_EQ(score, 1200);
  for (r = 0; r < BH; r++) for (c = 0; c < BW; c++) CHECK_EQ(board[r][c], 0);
}

TEST(rows_that_are_not_full_stay) {
  int c;
  clear_board();
  GLCD_Clear(Black);
  for (c = 0; c < BW - 1; c++) board[19][c] = Cyan;
  clear_lines();
  CHECK_EQ(lines, 0);
  CHECK_EQ(score, 0);
  CHECK_EQ(board[19][0], Cyan);
}

TEST(score_multiplies_by_level) {
  int c;
  clear_board();
  GLCD_Clear(Black);
  lines = 10; level = 1;
  for (c = 0; c < BW; c++) board[19][c] = Blue;
  clear_lines();
  CHECK_EQ(score, 80);
  CHECK_EQ(level, 1);
}

TEST(level_goes_up_every_ten_lines) {
  int r, c;
  clear_board();
  GLCD_Clear(Black);
  lines = 8;
  for (r = 18; r < 20; r++) for (c = 0; c < BW; c++) board[r][c] = Blue;
  clear_lines();
  CHECK_EQ(lines, 10);
  CHECK_EQ(level, 1);
}

TEST(locked_piece_is_written_into_the_board) {
  clear_board();
  pt = 3; pr = 0; px = 4; py = 18;
  lock_piece();
  CHECK_EQ(board[18][4], Yellow);
  CHECK_EQ(board[19][5], Yellow);
  CHECK_EQ(board[17][4], 0);
}

TEST(drawn_cells_match_the_board) {
  clear_board();
  GLCD_Clear(Black);
  board[19][0] = Red;
  draw_board();
  CHECK_EQ(fb[Y0 + 19 * CS][X0], Red);
  CHECK_EQ(fb[Y0 + 18 * CS][X0], Black);
}

TEST(bot_plays_a_full_game_and_clears_lines) {
  fake_buttons = tetris_bot;
  bot_quit_at = 0;
  fake_clock_start(10);
  bot_quit_at = fake_now() + 60000;
  tetris_run();
  fake_clock_stop();
  printf("  bot cleared %lu lines, score %lu\n", (unsigned long)lines, (unsigned long)score);
  CHECK(lines >= 3);
  CHECK(score > 0);
}

int main(void) {
  RUN(every_shape_has_four_cells_in_every_rotation);
  RUN(o_piece_looks_the_same_after_rotating);
  RUN(walls_and_floor_block_moves);
  RUN(stacked_blocks_block_moves);
  RUN(piece_can_hang_above_the_top);
  RUN(one_full_row_clears_and_scores_40);
  RUN(four_rows_at_once_score_1200);
  RUN(rows_that_are_not_full_stay);
  RUN(score_multiplies_by_level);
  RUN(level_goes_up_every_ten_lines);
  RUN(locked_piece_is_written_into_the_board);
  RUN(drawn_cells_match_the_board);
  RUN(bot_plays_a_full_game_and_clears_lines);
  DONE();
}
