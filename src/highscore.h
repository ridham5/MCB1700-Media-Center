#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdint.h>

#define HS_TETRIS 0
#define HS_SNAKE  1

uint32_t hs_get(int game);
int hs_submit(int game, uint32_t score);

const void *hs_flash_addr(void);
int hs_flash_write(const void *src, uint32_t len);

#endif
