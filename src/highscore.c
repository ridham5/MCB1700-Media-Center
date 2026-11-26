#include <string.h>
#include "highscore.h"

#define HS_MAGIC 0x48534331u

typedef struct {
  uint32_t magic;
  uint32_t best[2];
  uint32_t spare[61];
} HsBlock;

static HsBlock cache;
static int loaded;

static void load(void) {
  const HsBlock *f = (const HsBlock *)hs_flash_addr();
  if (f->magic == HS_MAGIC) {
    cache = *f;
  } else {
    memset(&cache, 0, sizeof cache);
    cache.magic = HS_MAGIC;
  }
  loaded = 1;
}

uint32_t hs_get(int game) {
  if (!loaded) load();
  return cache.best[game];
}

int hs_submit(int game, uint32_t score) {
  if (!loaded) load();
  if (score <= cache.best[game]) return 0;
  cache.best[game] = score;
  hs_flash_write(&cache, sizeof cache);
  return 1;
}
