#ifndef HOST_LPC17XX_H
#define HOST_LPC17XX_H

#include <stdint.h>

typedef struct { volatile uint32_t TCR; } HOST_TIM_TypeDef;
typedef struct { volatile uint32_t CR; } HOST_DAC_TypeDef;
typedef enum { USB_IRQn = 24, TIMER0_IRQn = 1 } IRQn_Type;

extern HOST_TIM_TypeDef host_tim0;
extern HOST_DAC_TypeDef host_dac;
#define LPC_TIM0 (&host_tim0)
#define LPC_DAC  (&host_dac)

extern uint32_t SystemFrequency;
uint32_t SysTick_Config(uint32_t ticks);
void NVIC_DisableIRQ(IRQn_Type irq);

#endif
