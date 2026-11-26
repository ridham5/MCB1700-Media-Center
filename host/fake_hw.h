#ifndef FAKE_HW_H
#define FAKE_HW_H

#include <stdint.h>

#define LCD_W 320
#define LCD_H 240

extern uint16_t fb[LCD_H][LCD_W];
extern unsigned int fake_leds;
extern uint32_t (*fake_buttons)(void);
extern uint32_t fake_stop_at;

void fake_clock_start(unsigned int us_per_ms);
void fake_clock_stop(void);
void fake_capture(const char *path, uint32_t every_ms);
void fake_capture_flush(void);
uint32_t fake_now(void);

#endif
