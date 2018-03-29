#include "./usart/debug_usart.h"
#include <stdio.h>

static volatile uint32_t  UARTTimeout = UART_WAIT_TIME;

static void NVIC_Configuration(void){
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	/* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	/* 抢断优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* 子优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void debug_usart_init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd( UART_1_RX_GPIO_CLK|UART_1_TX_GPIO_CLK, ENABLE);

	// 失能 UART 时钟
	USART_DeInit(UART_1);

	// 配置 Rx 引脚为复用功能
	GPIO_InitStructure.GPIO_Pin = UART_1_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// 浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART_1_RX_GPIO_PORT, &GPIO_InitStructure);

	// 配置 Tx 引脚为复用功能
	GPIO_InitStructure.GPIO_Pin = UART_1_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		// 复用开漏
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UART_1_TX_GPIO_PORT, &GPIO_InitStructure);

	// 使能 UART 时钟
	RCC_APB2PeriphClockCmd(UART_1_CLK, ENABLE);

	// 配置串口模式
	USART_InitStructure.USART_BaudRate = UART_1_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART_1, &USART_InitStructure);

	// 嵌套向量中断控制器NVIC配置
	NVIC_Configuration();

	// 使能串口接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	// 【注意！！！！！！！！！！！！！！！！！！！！！】【加了帧中断，上位机就不能实时的收到数据了！！！！！！！！！】
	// 下面这行函数在最初始的版本中是有的，但是在 ucos 版本中没有。
	// 启动【串口帧中断】
	// USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

	// 使能串口
	USART_Cmd(UART_1, ENABLE);

}



// 发送一个字节
uint16_t Usart_SendByte( USART_TypeDef * pUSARTx, u8 ch ){
	UARTTimeout = UART_WAIT_TIME;
	// 发送一个字节数据到USART1
	USART_SendData(pUSARTx,ch);

	// 等待发送完毕
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET){
		if(UARTTimeout-- == 0) return uart_error();
	}
	return 0;
}

// 发送一个字节，使用地址发送
void pUsart_SendByte( USART_TypeDef * pUSARTx, u8* ch ){
	// 发送一个字节数据到USART1
	USART_SendData(pUSARTx,*ch);
	// 发送
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}
// 一次性发送两个字节（也就是一条信息）,使用地址发送
// pUSARTx:		串口号
// message:		需要发送的16位信息
void pUsart_SentMessage( USART_TypeDef * pUSARTx, u16 * message){
	u8 temp_h, temp_l;

	// 取出高八位
	temp_h = ((*message)&0XFF00)>>8;
	// 取出低八位
	temp_l = (*message)&0XFF;
	// 发送高八位
	USART_SendData(pUSARTx,temp_h);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	// 发送低八位
	USART_SendData(pUSARTx,temp_l);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}


// 接收一个字节
uint16_t Usart_RecvByte( USART_TypeDef * pUSARTx){
	// UARTTimeout = UART_WAIT_TIME;
	UARTTimeout = 1000;
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_RXNE) == RESET){
		if(UARTTimeout-- == 0) return 0xFFFF;
	}
	return USART_ReceiveData(pUSARTx)&0x00FF;
}

// 预留，串口等待超时
uint16_t uart_error(void){
	return 0xFF;
}


// 重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f){
	// 发送一个字节数据到串口
	USART_SendData(UART_1, (uint8_t) ch);

	// 等待发送完毕
	while (USART_GetFlagStatus(UART_1, USART_FLAG_TXE) == RESET);

	return (ch);
}


// 重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f){
	// 等待串口输入数据
	while (USART_GetFlagStatus(UART_1, USART_FLAG_RXNE) == RESET);
	return (int)USART_ReceiveData(UART_1);
}
