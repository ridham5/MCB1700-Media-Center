#ifndef SNAKE_BOT_H
#define SNAKE_BOT_H

#include <stdlib.h>

static uint32_t snake_quit_at;

static int snake_safe(int x, int y) {
  int i;
  if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return 0;
  for (i = 0; i < len - 1; i++) if (sx[i] == x && sy[i] == y) return 0;
  return 1;
}

static uint32_t snake_bot(void) {
  static const int dxs[4] = { 1, -1, 0, 0 }, dys[4] = { 0, 0, 1, -1 };
  static const uint32_t keys[4] = { KBD_RIGHT, KBD_LEFT, KBD_DOWN, KBD_UP };
  uint32_t t = fake_now();
  int d, best = -1, bestd = 1 << 30, dist;
  if (snake_quit_at && t >= snake_quit_at) return ((t / 40) & 1) ? KBD_SELECT : 0;
  for (d = 0; d < 4; d++) {
    int nx = sx[0] + dxs[d], ny = sy[0] + dys[d];
    if (!snake_safe(nx, ny)) continue;
    dist = abs(nx - fx) + abs(ny - fy);
    if (dist < bestd) { bestd = dist; best = d; }
  }
  return best < 0 ? 0 : keys[best];
}

#endif
