#include "common.h"
#ifdef HIGHSCORES
#include "highscore.h"
#endif

#define BW 10
#define BH 20
#define CS 11
#define X0 20
#define Y0 10

static uint16_t board[BH][BW];

static const uint16_t shapes[7][4] = {
  { 0x0F00, 0x2222, 0x00F0, 0x4444 },
  { 0x44C0, 0x8E00, 0x6440, 0x0E20 },
  { 0x4460, 0x0E80, 0xC440, 0x2E00 },
  { 0xCC00, 0xCC00, 0xCC00, 0xCC00 },
  { 0x06C0, 0x8C40, 0x6C00, 0x4620 },
  { 0x0E40, 0x4C40, 0x4E00, 0x4640 },
  { 0x0C60, 0x4C80, 0xC600, 0x2640 }
};
static const uint16_t colors[7] = { Cyan, Blue, ORANGE, Yellow, Green, Magenta, Red };

static int px, py, pr, pt;
static uint32_t score, lines, level;

static int cell_on(int t, int r, int row, int col) {
  return (shapes[t][r] >> (15 - (row * 4 + col))) & 1;
}

static void draw_cell(int col, int row, uint16_t c) {
  if (row < 0) return;
  fill_rect(X0 + col * CS, Y0 + row * CS, CS - 1, CS - 1, c ? c : Black);
}

static int fits(int t, int r, int x, int y) {
  int i, j, bx, by;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      if (cell_on(t, r, i, j)) {
        bx = x + j; by = y + i;
        if (bx < 0 || bx >= BW || by >= BH) return 0;
        if (by >= 0 && board[by][bx]) return 0;
      }
  return 1;
}

static void draw_piece(int show) {
  int i, j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      if (cell_on(pt, pr, i, j))
        draw_cell(px + j, py + i, show ? colors[pt] : 0);
}

static void draw_board(void) {
  int r, c;
  for (r = 0; r < BH; r++)
    for (c = 0; c < BW; c++)
      draw_cell(c, r, board[r][c]);
}

static void draw_stats(void) {
  char buf[16];
  sprintf(buf, "%-7lu", (unsigned long)score);
  text(3, 10, 1, White, Black, buf);
  sprintf(buf, "%-7lu", (unsigned long)lines);
  text(5, 10, 1, White, Black, buf);
  sprintf(buf, "%-7lu", (unsigned long)level + 1);
  text(7, 10, 1, White, Black, buf);
#ifdef HIGHSCORES
  sprintf(buf, "Best %-4lu", (unsigned long)hs_get(HS_TETRIS));
  text(8, 10, 1, Yellow, Black, buf);
#endif
}

static int try_move(int dx, int dy, int dr) {
  int nr = (pr + dr) & 3;
  if (!fits(pt, nr, px + dx, py + dy)) return 0;
  draw_piece(0);
  px += dx; py += dy; pr = nr;
  draw_piece(1);
  return 1;
}

static void clear_lines(void) {
  static const uint32_t pts[5] = { 0, 40, 100, 300, 1200 };
  int r, c, rr, n = 0, full;
  for (r = BH - 1; r >= 0; r--) {
    full = 1;
    for (c = 0; c < BW; c++) if (!board[r][c]) { full = 0; break; }
    if (full) {
      for (rr = r; rr > 0; rr--)
        for (c = 0; c < BW; c++) board[rr][c] = board[rr - 1][c];
      for (c = 0; c < BW; c++) board[0][c] = 0;
      n++;
      r++;
    }
  }
  if (n) {
    score += pts[n] * (level + 1);
    lines += n;
    level = lines / 10;
    draw_board();
    draw_stats();
  }
}

static int spawn(void) {
  pt = rnd() % 7;
  pr = 0; px = 3; py = 0;
  if (!fits(pt, pr, px, py)) return 0;
  draw_piece(1);
  return 1;
}

static void lock_piece(void) {
  int i, j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      if (cell_on(pt, pr, i, j) && py + i >= 0)
        board[py + i][px + j] = colors[pt];
}

void tetris_run(void) {
  int r, c;
  uint32_t k, held, next_fall, interval, next_rep = 0;

  for (r = 0; r < BH; r++) for (c = 0; c < BW; c++) board[r][c] = 0;
  score = lines = level = 0;
  rnd_seed(msTicks);

  GLCD_Clear(Black);
  fill_rect(X0 - 3, Y0 - 3, BW * CS + 5, BH * CS + 5, DarkGrey);
  fill_rect(X0 - 1, Y0 - 1, BW * CS + 1, BH * CS + 1, Black);
  text(0, 10, 1, Yellow, Black, "TETRIS");
  text(2, 10, 1, LightGrey, Black, "Score");
  text(4, 10, 1, LightGrey, Black, "Lines");
  text(6, 10, 1, LightGrey, Black, "Level");
  text(27, 27, 0, LightGrey, Black, "UP=rotate SEL=quit");
  draw_stats();
  LED_Out(0x04);

  spawn();
  next_fall = msTicks + 600;

  while (1) {
    k = joy_pressed();
    held = joy_held();
    if (k & KBD_SELECT) return;
    if (k & KBD_UP) try_move(0, 0, 1);
    if (k & KBD_LEFT)  { try_move(-1, 0, 0); next_rep = msTicks + 200; }
    if (k & KBD_RIGHT) { try_move(1, 0, 0);  next_rep = msTicks + 200; }
    if ((held & (KBD_LEFT | KBD_RIGHT)) && msTicks >= next_rep) {
      try_move((held & KBD_LEFT) ? -1 : 1, 0, 0);
      next_rep = msTicks + 80;
    }

    interval = 600 - (level > 9 ? 9 : level) * 55;
    if (held & KBD_DOWN) interval = 50;

    if (msTicks >= next_fall) {
      next_fall = msTicks + interval;
      if (!try_move(0, 1, 0)) {
        lock_piece();
        clear_lines();
        if (!spawn()) break;
      }
    }
    delay_ms(10);
  }

  fill_rect(X0, Y0 + 80, BW * CS, 60, Black);
  text(4, 2, 1, Red, Black, "GAME");
  text(5, 2, 1, Red, Black, "OVER");
#ifdef HIGHSCORES
  if (hs_submit(HS_TETRIS, score)) text(7, 1, 1, Yellow, Black, "NEW BEST");
#endif
  text(9, 10, 1, White, Black, "SEL=menu");
  while (!(joy_pressed() & KBD_SELECT)) delay_ms(20);
}
