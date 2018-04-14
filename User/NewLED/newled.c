#include "./NewLED/newled.h"
#include "./Delay/delay.h"

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

// 开门灯效(漩涡灯效)
void LED_OpenDoor(void) {
    LED5_OFF();		delay_ms(30);
    LED8_OFF();		delay_ms(30);
    LED7_OFF();		delay_ms(30);
    LED4_OFF();		delay_ms(30);

    LED1_OFF();		delay_ms(30);   LED5_ON();
    LED2_OFF();		delay_ms(30);   LED8_ON();
    LED3_OFF();		delay_ms(30);   LED7_ON();
    LED6_OFF();		delay_ms(30);   LED4_ON();
    LED9_OFF();		delay_ms(30);   LED1_ON();
    LED12_OFF();	delay_ms(30);   LED2_ON();
    LED11_OFF();	delay_ms(30);   LED3_ON();
    LED10_OFF();	delay_ms(30);   LED6_ON();
    LED7_OFF();		delay_ms(30);   LED9_ON();
	LED4_OFF();		delay_ms(30);   LED12_ON();
	LED1_OFF();		delay_ms(30);   LED11_ON();
	LED2_OFF();		delay_ms(30);   LED10_ON();
	LED5_OFF();		delay_ms(30);   LED7_ON();

    LED4_ON();		delay_ms(30);
    LED1_ON();		delay_ms(30);
    LED2_ON();		delay_ms(30);
    LED5_ON();

}

// 开门验证失败灯效（叉叉灯效）
void LED_OpenError(void) {
	LED1_OFF();LED3_OFF();LED5_OFF();LED7_OFF();LED9_OFF();
	delay_ms(100);
	LED1_ON();LED3_ON();LED5_ON();LED7_ON();LED9_ON();
	
	delay_ms(100);

	LED1_OFF();LED3_OFF();LED5_OFF();LED7_OFF();LED9_OFF();
	delay_ms(100);
	LED1_ON();LED3_ON();LED5_ON();LED7_ON();LED9_ON();
}

// 全键盘灯由暗到亮
void LED_OFF2ON(void) {
	int count;
	for (u8 i=1; i<50; i++) {
		count = 0;
		while(1) {
			count++;
			led_on_all();
			delay_us(i);
			led_off_all();
			delay_us(50-i);
			if (count>=100-2*i) break;
		}
	}
	led_on_all();
}

// 全键盘灯由亮到暗
void LED_ON2OFF(void) {
	int count;
	for (u8 i=1; i<50; i++) {
		count = 0;
		while(1) {
			count++;
			led_off_all();
			delay_us(i);
			led_on_all();
			delay_us(50-i);
			if (count>=100-2*i) break;
		}
	}
	led_off_all();
}
