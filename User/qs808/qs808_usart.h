#ifndef __QS808_USART_H
#define	__QS808_USART_H
#include "stm32f10x_conf.h"
#include <stdio.h>
#include "my_board.h"


//引脚定义
/*******************************************************/
#define QS808_USART                             USART2
#define QS808_USART_CLK                         RCC_APB1Periph_USART2
#define QS808_USART_BAUDRATE                    115200  //串口波特率

#define QS808_USART_IRQn									USART2_IRQn
#define QS808_USART_RX_ISR								USART2_IRQHandler

#define QS808_TX_GPIO_PORT                GPIOA
#define QS808_TX_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define QS808_TX_PIN                      GPIO_Pin_2


#define QS808_RX_GPIO_PORT                GPIOA
#define QS808_RX_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define QS808_RX_PIN                      GPIO_Pin_3


#define QS808_NRST_GPIO_PORT                GPIOA
#define QS808_NRST_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define QS808_NRST_PIN                      GPIO_Pin_1

#define QS808_INT_GPIO_PORT                GPIOB
#define QS808_INT_GPIO_CLK                 RCC_APB2Periph_GPIOB
#define QS808_INT_PIN                      GPIO_Pin_0

#define QS808_INT_PIN_SOURCE_PORT					 GPIO_PortSourceGPIOB
#define QS808_INT_PIN_SOURCE_PIN					 GPIO_PinSource0
#define QS808_INT_EXT_LINE								 EXTI_Line0

#define QS808_INT_EXT_IRQHandler 					 EXTI0_IRQHandler
#define QS808_INT_IRQn										 EXTI0_IRQn

#define QS808_NRST_High() 						GPIO_SetBits(QS808_NRST_GPIO_PORT,QS808_NRST_PIN)
#define QS808_NRST_Low() 							GPIO_ResetBits(QS808_NRST_GPIO_PORT,QS808_NRST_PIN)

#define QS808_DMA_RCC                 		RCC_AHBPeriph_DMA1
#define  QS808_USART_DMA_Channel 			DMA1_Channel6
#define QS808_USART_DMA_TCx						DMA1_FLAG_TC6
void QS808_UART_Init(void);
//uint16_t Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch );
//uint16_t Usart_RecvByte( USART_TypeDef * pUSARTx);
//uint16_t uart_error(void);
#endif
