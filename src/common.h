#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include "LPC17xx.h"
#include "GLCD.h"
#include "KBD.h"
#include "LED.h"

#define ORANGE 0xFD20

extern volatile uint32_t msTicks;

void delay_ms(uint32_t ms);
uint32_t joy_pressed(void);
uint32_t joy_held(void);
void joy_wait_release(void);
void rnd_seed(uint32_t s);
uint32_t rnd(void);
void fill_rect(int x, int y, int w, int h, uint16_t c);
void text(int ln, int col, int big, uint16_t fg, uint16_t bg, const char *s);
void lcd_lock(void);
void lcd_unlock(void);

void gallery_run(void);
void audio_run(void);
void tetris_run(void);
void snake_run(void);

#endif
