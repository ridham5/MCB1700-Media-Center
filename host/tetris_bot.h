#ifndef TETRIS_BOT_H
#define TETRIS_BOT_H

#include <stdlib.h>
#include <string.h>

static int bot_tx, bot_tr, bot_piece_id = -1, bot_last_py = 99;
static uint32_t bot_quit_at;

static int bot_eval(uint16_t b[BH][BW]) {
  int c, r, h[BW], holes = 0, agg = 0, bump = 0, lines = 0, full;
  for (r = 0; r < BH; r++) {
    full = 1;
    for (c = 0; c < BW; c++) if (!b[r][c]) full = 0;
    lines += full;
  }
  for (c = 0; c < BW; c++) {
    h[c] = 0;
    for (r = 0; r < BH; r++) if (b[r][c]) { h[c] = BH - r; break; }
    for (r = BH - h[c]; r < BH; r++) if (!b[r][c]) holes++;
    agg += h[c];
    if (c) bump += abs(h[c] - h[c - 1]);
  }
  return -51 * agg + 76 * lines * 10 - 36 * holes * 10 - 18 * bump;
}

static void bot_plan(void) {
  static uint16_t copy[BH][BW];
  int r, x, y, i, j, best = -2147483647, s;
  for (r = 0; r < 4; r++)
    for (x = -3; x < BW; x++) {
      if (!fits(pt, r, x, py)) continue;
      y = py;
      while (fits(pt, r, x, y + 1)) y++;
      memcpy(copy, board, sizeof board);
      for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
          if (cell_on(pt, r, i, j) && y + i >= 0) copy[y + i][x + j] = 1;
      s = bot_eval(copy);
      if (s > best) { best = s; bot_tx = x; bot_tr = r; }
    }
}

static uint32_t tetris_bot(void) {
  uint32_t t = fake_now(), k = 0;
  if (bot_quit_at && t >= bot_quit_at) return ((t / 40) & 1) ? KBD_SELECT : 0;
  if (py < bot_last_py) { bot_piece_id++; bot_plan(); }
  bot_last_py = py;
  if (pr != bot_tr) return ((t / 30) & 1) ? KBD_UP : 0;
  if (px > bot_tx) return KBD_LEFT;
  if (px < bot_tx) return KBD_RIGHT;
  k = KBD_DOWN;
  return k;
}

#endif
