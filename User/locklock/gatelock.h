#ifndef	__GATELOCK_H
#define	__GATELOCK_H

#include "stm32f10x.h"
#include "sys.h"
#include "./Delay/delay.h"

#define L9LL0S_A_GPIO_PORT                GPIOB
#define L9LL0S_A_GPIO_CLK                 RCC_APB2Periph_GPIOB
#define L9LL0S_A_PIN                      GPIO_Pin_6

#define L9LL0S_B_GPIO_PORT                GPIOB
#define L9LL0S_B_GPIO_CLK                 RCC_APB2Periph_GPIOB
#define L9LL0S_B_PIN                      GPIO_Pin_7

#define motor_clockwise()					GPIO_SetBits(L9LL0S_A_GPIO_PORT,L9LL0S_A_PIN);GPIO_ResetBits(L9LL0S_B_GPIO_PORT,L9LL0S_B_PIN);
#define motor_anti_clockwise() 		GPIO_SetBits(L9LL0S_B_GPIO_PORT,L9LL0S_B_PIN);GPIO_ResetBits(L9LL0S_A_GPIO_PORT,L9LL0S_A_PIN);
#define motor_hold_on()							GPIO_ResetBits(L9LL0S_A_GPIO_PORT,L9LL0S_A_PIN);GPIO_ResetBits(L9LL0S_B_GPIO_PORT,L9LL0S_B_PIN);
void Gate_Init(void);			//解锁房门
void Gate_Unlock(void);			//门锁 IO 初始化


#endif
