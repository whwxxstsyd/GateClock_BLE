#include "stm32f10x.h"
#include "./led/led.h"

void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	//使能GPIOB时钟
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOA时钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//重映射功能，将jtag 的引脚屏蔽
	
	GPIO_InitStructure.GPIO_Pin = LED1_PIN|LED2_PIN|LED4_PIN|LED5_PIN|LED7_PIN|LED9_PIN|LED10_PIN|LED0_PIN|LED11_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = LED3_PIN|LED6_PIN|LED8_PIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void led_on_all(void)
{
		LED1_ON();
		LED2_ON();		
		LED3_ON();		
		LED4_ON();		
		LED5_ON();		
		LED6_ON();		
		LED7_ON();		
		LED8_ON();		
		LED9_ON();		
		LED10_ON();		
		LED0_ON();		
		LED11_ON();
}

void led_off_all(void)
{
		LED1_OFF();
		LED2_OFF();		
		LED3_OFF();		
		LED4_OFF();		
		LED5_OFF();		
		LED6_OFF();		
		LED7_OFF();		
		LED8_OFF();		
		LED9_OFF();		
		LED10_OFF();		
		LED0_OFF();		
		LED11_OFF();	
}

