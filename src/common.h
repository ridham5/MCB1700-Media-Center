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

/* Volume bar on the MP3 player screen (RTX build) */
#define VOL_BAR_X 16
#define VOL_BAR_Y 196
#define VOL_BAR_W 288
#define VOL_BAR_H 12

void audio_start(void);
void audio_stop(void);
#ifdef USE_RTX
void run_game(void (*fn)(void));
void rtx_audio(int on);
#endif

void gallery_run(void);
void audio_run(void);
void tetris_run(void);
void snake_run(void);

#endif
