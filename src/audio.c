#include "common.h"

extern void USB_Connect(uint32_t con);

/* Disconnect USB and stop the sample timer so the audio stops cleanly and can be re-entered. */
void audio_stop(void) {
  USB_Connect(0);
  NVIC_DisableIRQ(USB_IRQn);
  NVIC_DisableIRQ(TIMER0_IRQn);
  LPC_TIM0->TCR = 0;
  LPC_DAC->CR = 0x00008000;
}

void audio_run(void) {
  GLCD_Clear(Black);
  fill_rect(0, 0, 320, 48, Maroon);
  text(0, 0, 1, White, Maroon, "     MP3 PLAYER     ");
  text(3, 1, 1, Green, Black, "Streaming from PC");
  text(4, 1, 1, Green, Black, "over USB...");
  text(6, 1, 1, White, Black, "Knob = volume");
  text(9, 1, 1, LightGrey, Black, "SELECT = back");
  LED_Out(0x81);

#ifdef USE_RTX
  rtx_audio(1);                          /* audio thread starts the stream; volume thread draws the bar */
#else
  audio_start();
#endif

  while (!(joy_pressed() & KBD_SELECT)) delay_ms(20);

#ifdef USE_RTX
  rtx_audio(0);
#else
  audio_stop();
#endif
}
