#ifndef __TSM12_H
#define __TSM12_H
#include "stm32f10x.h"
#include "./Delay/delay.h"

// 引脚定义
#define TSM12_SDA_GPIO_PORT				GPIOB
#define TSM12_SDA_GPIO_CLK				RCC_APB2Periph_GPIOB
#define TSM12_SDA_PIN					GPIO_Pin_12
#define TSM12_SCL_GPIO_PORT				GPIOB
#define TSM12_SCL_GPIO_CLK				RCC_APB2Periph_GPIOB
#define TSM12_SCL_PIN					GPIO_Pin_13
#define TSM12_RST_GPIO_PORT				GPIOB	// 高复位
#define TSM12_RST_GPIO_CLK				RCC_APB2Periph_GPIOB
#define TSM12_RST_PIN					GPIO_Pin_15
#define TSM12_I2CEN_GPIO_PORT			GPIOA	// 低使能
#define	TSM12_I2CEN_GPIO_CLK			RCC_APB2Periph_GPIOA
#define TSM12_I2CEN_PIN					GPIO_Pin_8
#define TSM12_INT_GPIO_PORT				GPIOB
#define TSM12_INT_GPIO_CLK				RCC_APB2Periph_GPIOB
#define TSM12_INT_PIN					GPIO_Pin_14
#define TSM12_INT_PIN_SOURCE_PORT		GPIO_PortSourceGPIOB
#define TSM12_INT_PIN_SOURCE_PIN		GPIO_PinSource14
#define TSM12_INT_EXT_LINE				EXTI_Line14
#define TSM12_INT_EXT_IRQHandler		EXTI15_10_IRQHandler
#define TSM12_INT_IRQn					EXTI15_10_IRQn

#define Sens1							0x02
#define Sens2 							0x03
#define Sens3 							0x04
#define Sens4 							0x05
#define Sens5 							0x06
#define Sens6 							0x07
#define CTR1 							0x08
#define CTR2 							0x09
#define Ref_rst1 						0x0A
#define Ref_rst2 						0x0B
#define Ch_hold1 						0x0C
#define Ch_hold2 						0x0D
#define Cal_hold1 						0x0E
#define Cal_hold2 						0x0F
#define Output1 						0x10
#define Output2 						0x11
#define Output3 						0x12


void TSM12_Init(void);
uint8_t TMS12_ReadOnKey(void);
void TSM12_SLEEP(void);
void TSM12_Wakeup(void);

#endif
