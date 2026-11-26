#include "common.h"
#include "photo1.c"
#include "photo2.c"
#include "photo3.c"
#include "photo4.c"

#define FLIP_H 0

typedef struct {
  const unsigned char *data;
  int w, h;
  const char *name;
} Photo;

static const Photo photos[] = {
  { PHOTO1_PIXEL_DATA, PHOTO1_WIDTH, PHOTO1_HEIGHT, "Sunset" },
  { PHOTO2_PIXEL_DATA, PHOTO2_WIDTH, PHOTO2_HEIGHT, "Mountains" },
  { PHOTO3_PIXEL_DATA, PHOTO3_WIDTH, PHOTO3_HEIGHT, "City" },
  { PHOTO4_PIXEL_DATA, PHOTO4_WIDTH, PHOTO4_HEIGHT, "Ocean" }
};
#define N_PHOTOS (sizeof(photos) / sizeof(photos[0]))

static unsigned short rowbuf[320];

static void show(int i) {
  const Photo *p = &photos[i];
  const unsigned short *px = (const unsigned short *)p->data;
  int x0 = (320 - p->w) / 2, y0 = 36, r, c;
  char buf[32];

  fill_rect(0, 24, 320, 192, Black);
  for (r = 0; r < p->h; r++) {
    for (c = 0; c < p->w; c++)
      rowbuf[c] = FLIP_H ? px[r * p->w + (p->w - 1 - c)] : px[r * p->w + c];
    GLCD_Bitmap(x0, y0 + r, p->w, 1, (unsigned char *)rowbuf);
  }
  sprintf(buf, "%d/%d %-12s", i + 1, (int)N_PHOTOS, p->name);
  text(9, 1, 1, White, Black, buf);
  LED_Out(1 << i);
}

void gallery_run(void) {
  int i = 0;
  uint32_t k;

  GLCD_Clear(Black);
  text(0, 0, 1, Yellow, Black, "  PHOTO GALLERY     ");
  show(i);
  while (1) {
    k = joy_pressed();
    if (k & KBD_SELECT) return;
    if (k & KBD_RIGHT) { i = (i + 1) % N_PHOTOS; show(i); }
    if (k & KBD_LEFT)  { i = (i + N_PHOTOS - 1) % N_PHOTOS; show(i); }
    delay_ms(20);
  }
}
