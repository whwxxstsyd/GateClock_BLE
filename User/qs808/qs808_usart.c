#include "./qs808/qs808_usart.h"
#define UART_WAIT_TIME								0xfffff//串口等待时间

static volatile uint32_t  UARTTimeout = UART_WAIT_TIME;
static void NVIC_Configuration(void) {
	NVIC_InitTypeDef NVIC_InitStructure;
	/* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = QS808_USART_IRQn;
	/* 抢断优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* 子优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
}


void QS808_UART_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd( QS808_RX_GPIO_CLK|QS808_TX_GPIO_CLK, ENABLE);
	USART_DeInit(QS808_USART);

	/*uart1 rx*/
	GPIO_InitStructure.GPIO_Pin = QS808_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(QS808_RX_GPIO_PORT, &GPIO_InitStructure);

	/*uart1 tx*/
	GPIO_InitStructure.GPIO_Pin = QS808_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(QS808_TX_GPIO_PORT, &GPIO_InitStructure);

	/* 使能 UART 时钟 */
	RCC_APB1PeriphClockCmd(QS808_USART_CLK, ENABLE);

	/* 配置串口 模式 */
	USART_InitStructure.USART_BaudRate = QS808_USART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(QS808_USART, &USART_InitStructure);

	NVIC_Configuration();
	USART_ITConfig(QS808_USART, USART_IT_RXNE, ENABLE);
	USART_Cmd(QS808_USART, ENABLE);
}
