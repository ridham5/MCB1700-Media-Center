#include "LPC17xx.h"
#include "highscore.h"

#define HS_SECTOR    29
#define HS_ADDR      0x00078000u
#define IAP_LOCATION 0x1FFF1FF1u

#define IAP_PREPARE  50
#define IAP_COPY     51
#define IAP_ERASE    52

extern uint32_t SystemFrequency;

typedef void (*IapEntry)(unsigned int *in, unsigned int *out);

static unsigned int iap(unsigned int cmd, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4) {
  unsigned int in[5], out[5];
  in[0] = cmd; in[1] = p1; in[2] = p2; in[3] = p3; in[4] = p4;
  ((IapEntry)IAP_LOCATION)(in, out);
  return out[0];
}

const void *hs_flash_addr(void) { return (const void *)HS_ADDR; }

int hs_flash_write(const void *src, uint32_t len) {
  unsigned int r, khz = SystemFrequency / 1000;
  (void)len;
  __disable_irq();
  r = iap(IAP_PREPARE, HS_SECTOR, HS_SECTOR, 0, 0);
  if (!r) r = iap(IAP_ERASE, HS_SECTOR, HS_SECTOR, khz, 0);
  if (!r) r = iap(IAP_PREPARE, HS_SECTOR, HS_SECTOR, 0, 0);
  if (!r) r = iap(IAP_COPY, HS_ADDR, (unsigned int)src, 256, khz);
  __enable_irq();
  return (int)r;
}
