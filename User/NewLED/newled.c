#include "newled.h"

// led初始化
void NewLed_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	//使能GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = LED7_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOD, ENABLE );	//使能GPIOD时钟
	GPIO_InitStructure.GPIO_Pin = LED6_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC, ENABLE );	//使能GPIOC时钟
	GPIO_InitStructure.GPIO_Pin = LED1_PIN|LED2_PIN|LED3_PIN|LED4_PIN|LED5_PIN|LED8_PIN|LED9_PIN|LED10_PIN|LED11_PIN|LED12_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

// 打开所有的按键灯
void led_on_all(void) {
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
	LED11_ON();
	LED12_ON();
}

// 关闭所有的按键灯
void led_off_all(void) {
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
	LED11_OFF();
	LED12_OFF();
}
