#include "./BLE/BLE.h"
#include <string.h>					// 为了使用 strcat 函数
#include <stdio.h>					// 为了使用 printf 函数

uint8_t BLE_FRAME_REC_FLAG=0;
char BLE_FRAME[100];
int BLE_FRAME_INDEX=0;

void BLE_init(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( BLE_NRST_PORT_CLK|BLE_PWRC_PORT_CLK|BLE_INT_PORT_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	
	GPIO_InitStructure.GPIO_Pin = BLE_NRST_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BLE_NRST_PORT, &GPIO_InitStructure);
	

	GPIO_InitStructure.GPIO_Pin = BLE_PWRC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BLE_NRST_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = BLE_INT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BLE_INT_PORT, &GPIO_InitStructure);

	

	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
 	GPIO_EXTILineConfig(BLE_INT_PIN_SOURCE_PORT,BLE_INT_PIN_SOURCE_PIN);

	// 外部中断
	EXTI_InitStructure.EXTI_Line = BLE_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// 配置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = BLE_INT_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	// 蓝牙复位
	BLE_RST_OFF();

	// 关闭 AT 命令
	AT_MODE_OFF();

}

void BLE_USART_SendStr(char* str){
	printf("%s",str);
}

void BLE_USART_CMD(char* cmd){
	// 预分配20长度，这样strcat之后不会溢出
	char BLE_CMD[50]="AT+";
	char enter[] = "\r\n";
	char* str_pt = BLE_CMD;
	str_pt = strcat(str_pt,cmd);
	str_pt = strcat(str_pt,enter);
	BLE_USART_SendStr(str_pt);
}

// 蓝牙进入休眠模式
void BLE_Sleep(void) {
	;
}
