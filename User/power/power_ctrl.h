#ifndef __POWER_CTRL_H
#define	__POWER_CTRL_H
#include "stm32f10x_gpio.h"
#include "my_board.h"

#define POWER_CTRL_GPIO_PORT                GPIOB
#define POWER_CTRL_GPIO_CLK                 RCC_APB2Periph_GPIOB
#define POWER_CTRL_PIN                      GPIO_Pin_9


void power_ctrl_init(void);
void  PWR_Standby_Mode(void);
void Wkup_init(void);


#define Power_ctrl_off() 	GPIO_ResetBits(POWER_CTRL_GPIO_PORT,POWER_CTRL_PIN)
#define Power_ctrl_on() 	GPIO_SetBits(POWER_CTRL_GPIO_PORT,POWER_CTRL_PIN)
#endif
