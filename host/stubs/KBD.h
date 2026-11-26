#ifndef HOST_KBD_H
#define HOST_KBD_H

#include <stdint.h>

#define KBD_SELECT 0x01
#define KBD_UP     0x08
#define KBD_RIGHT  0x10
#define KBD_DOWN   0x20
#define KBD_LEFT   0x40
#define KBD_MASK   0x79

void KBD_Init(void);
uint32_t get_button(void);

#endif
