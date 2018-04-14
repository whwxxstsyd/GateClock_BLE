#ifndef __NEWLED_H
#define __NEWLED_H

#include "stm32f10x.h"


// 引脚定义
#define LED1_PORT				GPIOC
#define LED1_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED1_PIN				GPIO_Pin_7
#define LED1_OFF()				GPIO_SetBits(LED1_PORT,LED1_PIN);
#define LED1_ON()				GPIO_ResetBits(LED1_PORT,LED1_PIN);

#define LED2_PORT				GPIOC
#define LED2_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED2_PIN				GPIO_Pin_8
#define LED2_OFF()				GPIO_SetBits(LED2_PORT,LED2_PIN);
#define LED2_ON()				GPIO_ResetBits(LED2_PORT,LED2_PIN);

#define LED3_PORT				GPIOC
#define LED3_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED3_PIN				GPIO_Pin_9
#define LED3_OFF()				GPIO_SetBits(LED3_PORT,LED3_PIN);
#define LED3_ON()				GPIO_ResetBits(LED3_PORT,LED3_PIN);

#define LED4_PORT				GPIOC
#define LED4_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED4_PIN				GPIO_Pin_5
#define LED4_OFF()				GPIO_SetBits(LED4_PORT,LED4_PIN);
#define LED4_ON()				GPIO_ResetBits(LED4_PORT,LED4_PIN);

#define LED5_PORT				GPIOC
#define LED5_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED5_PIN				GPIO_Pin_12
#define LED5_OFF()				GPIO_SetBits(LED5_PORT,LED5_PIN);
#define LED5_ON()				GPIO_ResetBits(LED5_PORT,LED5_PIN);

#define LED6_PORT				GPIOD
#define LED6_GPIO_CLK			RCC_APB2Periph_GPIOD
#define LED6_PIN				GPIO_Pin_2
#define LED6_OFF()				GPIO_SetBits(LED6_PORT,LED6_PIN);
#define LED6_ON()				GPIO_ResetBits(LED6_PORT,LED6_PIN);

#define LED7_PORT				GPIOA
#define LED7_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED7_PIN				GPIO_Pin_4
#define LED7_OFF()				GPIO_SetBits(LED7_PORT,LED7_PIN);
#define LED7_ON()				GPIO_ResetBits(LED7_PORT,LED7_PIN);

#define LED8_PORT				GPIOC
#define LED8_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED8_PIN				GPIO_Pin_0
#define LED8_OFF()				GPIO_SetBits(LED8_PORT,LED8_PIN);
#define LED8_ON()				GPIO_ResetBits(LED8_PORT,LED8_PIN);

#define LED9_PORT				GPIOC
#define LED9_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED9_PIN				GPIO_Pin_13
#define LED9_OFF()				GPIO_SetBits(LED9_PORT,LED9_PIN);
#define LED9_ON()				GPIO_ResetBits(LED9_PORT,LED9_PIN);

#define LED10_PORT				GPIOC
#define LED10_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED10_PIN				GPIO_Pin_3
#define LED10_OFF()				GPIO_SetBits(LED10_PORT,LED10_PIN);
#define LED10_ON()				GPIO_ResetBits(LED10_PORT,LED10_PIN);

#define LED11_PORT				GPIOC
#define LED11_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED11_PIN				GPIO_Pin_2
#define LED11_OFF()				GPIO_SetBits(LED11_PORT,LED11_PIN);
#define LED11_ON()				GPIO_ResetBits(LED11_PORT,LED11_PIN);

#define LED12_PORT				GPIOC
#define LED12_GPIO_CLK			RCC_APB2Periph_GPIOC
#define LED12_PIN				GPIO_Pin_1
#define LED12_OFF()				GPIO_SetBits(LED12_PORT,LED12_PIN);
#define LED12_ON()				GPIO_ResetBits(LED12_PORT,LED12_PIN);






//函数原型
void NewLed_Init(void);
void led_on_all(void);
void led_off_all(void);
void LED_OpenDoor(void);
void LED_OpenError(void);
void LED_OFF2ON(void);
void LED_ON2OFF(void);



#endif
