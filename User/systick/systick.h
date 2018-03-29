#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

void SysTick_Init(void);
void Delay_us(__IO u32 nTime);

#define Delay_ms(x) Delay_us(100*x)	 //??ms
#define Delay_s(x)  Delay_ms(1000*x)	 //??s

#endif /* __SYSTICK_H */
