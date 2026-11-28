#include "LPC17xx.h"

/*
 * RTX build only: RTX uses SysTick for its scheduler, so the 1 ms game clock that Tetris and
 * Snake time their moves with runs on TIMER1 instead.
 */
extern volatile uint32_t msTicks;
extern uint32_t SystemFrequency;

void TIMER1_IRQHandler(void) {
  LPC_TIM1->IR = 1;                              /* clear the MR0 interrupt */
  msTicks++;
}

void game_clock_init(void) {
  LPC_SC->PCONP |= 1 << 2;                       /* power up TIMER1 */
  LPC_TIM1->TCR = 2;                             /* hold in reset while configuring */
  LPC_TIM1->PR = 0;
  LPC_TIM1->MR0 = SystemFrequency / 4 / 1000 - 1;   /* PCLK = CCLK/4 (reset default) -> 1 ms */
  LPC_TIM1->MCR = 3;                             /* interrupt and reset on MR0 */
  LPC_TIM1->TCR = 1;                             /* run */
  NVIC_EnableIRQ(TIMER1_IRQn);
}
