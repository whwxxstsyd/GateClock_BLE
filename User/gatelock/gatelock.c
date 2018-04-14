#include "./gatelock/gatelock.h"
#include "./NewLED/newled.h"

void Gate_Init(void) {
	// IO 初始化
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(L9LL0S_A_GPIO_CLK|L9LL0S_B_GPIO_CLK, ENABLE);	//打开相应的 IO 时钟
	GPIO_InitStructure.GPIO_Pin = L9LL0S_A_PIN;				//选中需要配置的 IO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度为50Mhz
	GPIO_Init(L9LL0S_A_GPIO_PORT, &GPIO_InitStructure);		//初始化
	GPIO_InitStructure.GPIO_Pin = L9LL0S_B_PIN;
	GPIO_Init(L9LL0S_B_GPIO_PORT, &GPIO_InitStructure);
}

void Gate_Unlock(void) {
	// 门锁【打开】
	motor_anti_clockwise();

	// 原来这里延时500ms
	// delay_ms(500);
	// 现在加入灯效，延时就可以不是500了
	LED_OpenDoor();	//灯效112ms，所以就不用500ms延时了
	delay_ms(388);


	motor_hold_on();
	// if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR();
	
	// 给2s进门时间
	// 之所以这么写，是因为用寄存器方式延时，24位延时寄存器能写入的最大数是2^24，用72M倒计时，大概可以延时1.5s,故不能写delay_ms(10000)
	delay_ms(1000);delay_ms(1000);	

	// 门锁【关闭】
	motor_clockwise();
	delay_ms(500);
	motor_hold_on();
}
