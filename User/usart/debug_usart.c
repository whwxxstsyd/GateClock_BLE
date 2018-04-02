#include "./usart/debug_usart.h"
#include "./data/data_def.h"		// 为了使用 Userid2Ascii 转换用户ID，以及一些宏定义
#include <stdio.h>					// 为了使用 fgetc 函数重定向 printf


static volatile uint32_t  UARTTimeout = UART_WAIT_TIME;	// 串口接收等待时长
u8 USART_RecvBuf[USART_RECVBUF_LENGTH];	// 串口接收数据缓存池
u8 USART1_RecvBuf_Length=0;				// 串口接收数据的总长度
u8 USART_Recv_Flag=0;					// 串口接收到数据标志位


/********************************************** 用户函数 *************************************************/

// 接收手机端命令
// result:	是否收到了有效的包头命令
u16 Usart_RecvOrder(USART_TypeDef* pUSARTx) {
	// 如果串口接收到数据
	if(USART_Recv_Flag==1) {
		// 清空标志位
		USART_Recv_Flag = 0;

		// 清空RecBuf内容长度
		USART1_RecvBuf_Length = 0;

		return SYS_RECV_ORDER;
	}
	return SYS_NO_ORDER;
}

// 发送【用户ID】
// pUSARTx:		串口号
// user_id:		用户id
// notes:		12=0x303132 格式
void Usart_SendUserId(USART_TypeDef* pUSARTx, u16 user_id) {
	u8 temp[3];
	Userid2Ascii(user_id,temp);

	for (u8 i=0; i<3; i++){
		pUsart_SendByte(pUSARTx, temp+i);
	}
}

// 发送【射频卡】录入成功数据包
// pUSARTx:		串口号
// user_id:		十进制用户id
void Usart_SendRFCard_ADD_Success(USART_TypeDef* pUSARTx, u16 user_id) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	temp.m_totalLength = 15;
	temp.m_cmdId = CMDID_ADD_RFCARD;
	temp.m_seq = 0x0000;
	temp.m_errorCode = ERROR_CODE_SUCCESS;

	// 发送包头数据
	for (u8 i=0; i<6; i++) {
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}

	// 发送用户 ID
	Usart_SendUserId(pUSARTx, user_id);
}

// 发送【射频卡】录入失败数据包
// pUSARTx:		串口号
void Usart_SendRFCard_ADD_Error(USART_TypeDef* pUSARTx) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	temp.m_totalLength = 12;
	temp.m_cmdId = CMDID_ADD_RFCARD;
	temp.m_seq = 0x0000;
	temp.m_errorCode = ERROR_CODE_TIMEOUT;

	// 发送包头数据
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}
}

// 发送【射频卡】删除成功数据包
// pUSARTx:		串口号
void Usart_SendRFCard_DEL_Success(USART_TypeDef* pUSARTx) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	temp.m_totalLength = 12;
	temp.m_cmdId = CMDID_DEL_RFCARD;
	temp.m_seq = 0x0000;
	temp.m_errorCode = ERROR_CODE_SUCCESS;

	// 发送包头数据
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}
}

// 发送【射频卡】删除失败数据包
// pUSARTx:		串口号
void Usart_SendRFCard_DEL_Error(USART_TypeDef* pUSARTx) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	temp.m_totalLength = 12;
	temp.m_cmdId = CMDID_DEL_RFCARD;
	temp.m_seq = 0x0000;
	temp.m_errorCode = ERROR_CODE_ERROR;

	// 发送包头数据
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}
}

// 发送【指纹】添加成功数据包
// pUSARTx:		串口号
// seq:			第几次采集成功
void Usart_SendFinger_ADD_Success(USART_TypeDef* pUSARTx, u16 seq, u16 user_id) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	if (seq<3)
		temp.m_totalLength = 12;
	else
		temp.m_totalLength = 15;
	temp.m_cmdId = CMDID_ADD_FINGER;
	temp.m_seq = seq;
	temp.m_errorCode = ERROR_CODE_SUCCESS;

	// 发送包头数据
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}

	// 发送用户ID
	if (seq==3) 
		Usart_SendUserId(pUSARTx, user_id);
}

// 发送【指纹】添加失败数据包
// pUSARTx:		串口号
// seq:			第几次采集失败
// errorcode:	错误原因
void Usart_SendFinger_ADD_Error(USART_TypeDef* pUSARTx, u16 seq, u16 errorcode) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	temp.m_totalLength = 12;
	temp.m_cmdId = CMDID_ADD_FINGER;
	temp.m_seq = seq;
	temp.m_errorCode = errorcode;

	// 发送包头数据
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}
}


// 发送【指纹】删除成功数据包
// pUSARTx:		串口号
void Usart_SendFinger_DEL_Success(USART_TypeDef* pUSARTx) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	temp.m_totalLength = 12;
	temp.m_cmdId = CMDID_DEL_FINGER;
	temp.m_seq = 0x0000;
	temp.m_errorCode = ERROR_CODE_SUCCESS;

	// 发送包头数据
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}
}

// 发送【指纹】删除失败数据包
// pUSARTx:		串口号
void Usart_SendFinger_DEL_Error(USART_TypeDef* pUSARTx) {
	// 定义包头数据
	BleDataHead temp;
	temp.m_magicCode = MAGICCODE;
	temp.m_version = VERSION;
	temp.m_totalLength = 12;
	temp.m_cmdId = CMDID_DEL_FINGER;
	temp.m_seq = 0x0000;
	temp.m_errorCode = ERROR_CODE_ERROR;

	// 发送包头数据
	for (u8 i=0; i<6; i++){
		pUsart_SentMessage(pUSARTx, (u16*)&temp+i);
	}
}










/********************************************** 底层函数 *************************************************/

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

// 发送一个字节，使用地址发送
void pUsart_SendByte( USART_TypeDef * pUSARTx, u8* ch ){
	// 发送一个字节数据到USART1
	USART_SendData(pUSARTx,*ch);
	// 发送
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}

// 一次性发送两个字节（也就是一条信息）,使用地址发送
void pUsart_SentMessage( USART_TypeDef * pUSARTx, u16 * message){
	// pUSARTx:		串口号
	// message:		需要发送的16位信息
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

// 发送一个字节
uint16_t Usart_SendByte( USART_TypeDef * pUSARTx, u8 ch ){
	UARTTimeout = UART_WAIT_TIME;
	// 发送一个字节数据到USART1
	USART_SendData(pUSARTx,ch);

	// 等待发送完毕
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET){
		if(UARTTimeout-- == 0) return 0xFFFF;
	}
	return 0;
}

// 接收一个字节，特别注意，串口寄存器只有低9位有效！！！！！
uint16_t Usart_RecvByte( USART_TypeDef * pUSARTx){

	// 【特别注意】：这个地方一定要用FFFFF这么大的数据，不能用1000这么小的
	UARTTimeout = UART_WAIT_TIME;
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_RXNE) == RESET){
		if(UARTTimeout-- == 0) return 0xFFFF;
	}
	return USART_ReceiveData(pUSARTx)&0x00FF;
}

// 配置串口中断优先级
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

// 串口初始化程序
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

	// 启动【串口帧中断】
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

	// 使能串口
	USART_Cmd(UART_1, ENABLE);

}
