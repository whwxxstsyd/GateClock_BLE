#ifndef __VCCADC_H
#define __VCCADC_H
#include "my_board.h"
#include "stm32f10x.h"
void VCC_Adc_Init(void);  //ADC通道初始化
float Get_Battery(void);
void VCC_Adc_Sleep(void);
#define VCC_ADC_GPIO_PORT                GPIOA
#define VCC_ADC_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define VCC_ADC_PIN                      GPIO_Pin_0

 
#endif
