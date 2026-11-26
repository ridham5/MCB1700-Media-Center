#include "common.h"

#define CS 16
#define COLS 20
#define ROWS 14
#define Y0 16
#define MAXLEN (COLS * ROWS)

static uint8_t sx[MAXLEN], sy[MAXLEN];
static int len, fx, fy;
static uint32_t score;

static void cell(int x, int y, uint16_t c) {
  fill_rect(x * CS + 1, Y0 + y * CS + 1, CS - 2, CS - 2, c);
}

static int on_snake(int x, int y, int upto) {
  int i;
  for (i = 0; i < upto; i++) if (sx[i] == x && sy[i] == y) return 1;
  return 0;
}

static void place_food(void) {
  do { fx = rnd() % COLS; fy = rnd() % ROWS; } while (on_snake(fx, fy, len));
  cell(fx, fy, Red);
}

static void draw_score(void) {
  char buf[40];
  sprintf(buf, "SNAKE   Score: %-5lu  SELECT=quit", (unsigned long)score);
  text(0, 1, 0, Yellow, DarkGrey, buf);
}

void snake_run(void) {
  int i, dx = 1, dy = 0, ndx = 1, ndy = 0, nx, ny, grow;
  uint32_t k, next_step, speed = 160;

  rnd_seed(msTicks);
  GLCD_Clear(Black);
  fill_rect(0, 0, 320, Y0, DarkGrey);
  score = 0;
  draw_score();
  LED_Out(0x08);

  len = 3;
  for (i = 0; i < len; i++) { sx[i] = 5 - i; sy[i] = 7; cell(sx[i], sy[i], Green); }
  place_food();
  next_step = msTicks + speed;

  while (1) {
    k = joy_pressed();
    if (k & KBD_SELECT) return;
    if ((k & KBD_UP)    && dy == 0) { ndx = 0;  ndy = -1; }
    if ((k & KBD_DOWN)  && dy == 0) { ndx = 0;  ndy = 1; }
    if ((k & KBD_LEFT)  && dx == 0) { ndx = -1; ndy = 0; }
    if ((k & KBD_RIGHT) && dx == 0) { ndx = 1;  ndy = 0; }

    if (msTicks >= next_step) {
      next_step = msTicks + speed;
      dx = ndx; dy = ndy;
      nx = sx[0] + dx; ny = sy[0] + dy;
      grow = (nx == fx && ny == fy);
      if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS || on_snake(nx, ny, grow ? len : len - 1)) break;

      if (!grow) cell(sx[len - 1], sy[len - 1], Black);
      else if (len < MAXLEN) len++;
      for (i = len - 1; i > 0; i--) { sx[i] = sx[i - 1]; sy[i] = sy[i - 1]; }
      sx[0] = nx; sy[0] = ny;
      cell(sx[1], sy[1], Green);
      cell(nx, ny, Yellow);

      if (grow) {
        score += 10;
        draw_score();
        if (speed > 70) speed -= 5;
        place_food();
      }
    }
    delay_ms(10);
  }

  text(4, 5, 1, Red, Black, "GAME OVER");
  text(6, 3, 1, White, Black, "SELECT = menu");
  while (!(joy_pressed() & KBD_SELECT)) delay_ms(20);
}
