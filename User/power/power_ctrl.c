#include "./power/power_ctrl.h"
#include "stm32f10x_pwr.h"

// 外设电源控制	0表示正常工作，待机时把这个置位1
uint8_t WAKEUP_FLAG=0;

void power_ctrl_init(void)
{
	int i;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	POWER_CTRL_GPIO_CLK, ENABLE );

	GPIO_InitStructure.GPIO_Pin = POWER_CTRL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(POWER_CTRL_GPIO_PORT, &GPIO_InitStructure);
	Power_ctrl_off();
	for (i=0;i<999;i++);
}

// 进入待机模式
void PWR_Standby_Mode(void) {
	uint8_t cnt = 0;
	
	GPIO_InitTypeDef GPIO_InitStructure;

	// 原来的板子是 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	// 没用到的引脚
	// 现在的板子上 GPIO_Pin_4 用在了灯上面
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	// 没用到的引脚
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	delay_ms(1000);		// 防止其他通信进行时进入休眠
	BLE_Sleep();		// 【蓝牙】休眠
	VCC_Adc_Sleep();	// 【ADC】休眠
	RC522_SLEEP();		// 【射频卡】休眠
	TSM12_SLEEP();		// 【键盘】休眠
	Power_ctrl_off();	// 【音频led】断电
	OLED_OFF();			// 【OLED】休眠

	// qs808休眠
	while((QS808_STANDBY() != ERR_SUCCESS) && (cnt < 5)) {
		QS808_Rec_Buf_refresh();
		delay_ms(10);
		cnt++;
	}
	WAKEUP_FLAG = 1;

	PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
}

// 从待机模式中唤醒
void Wkup_init(void) {
	int i;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	// PA0引脚接到了TTP229的SDO引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);

	for(i=0;i<999;i++);
}
