#ifndef __DEBUG_USART_H
#define	__DEBUG_USART_H
#include "stm32f10x.h"

// 串口等待时间
#define UART_WAIT_TIME                     0xfffff

/********************************************** 引脚定义 *************************************************/
#define UART_1                             USART1
#define UART_1_CLK                         RCC_APB2Periph_USART1
#define UART_1_BAUDRATE                    115200

#define UART_1_RX_GPIO_PORT                GPIOA
#define UART_1_RX_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define UART_1_RX_PIN                      GPIO_Pin_10
#define UART_1_RX_AF                       GPIO_AF_USART1
#define UART_1_RX_SOURCE                   GPIO_PinSource10

#define UART_1_TX_GPIO_PORT                GPIOA
#define UART_1_TX_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define UART_1_TX_PIN                      GPIO_Pin_9
#define UART_1_TX_AF                       GPIO_AF_USART1
#define UART_1_TX_SOURCE                   GPIO_PinSource9





/********************************************** 用户函数 *************************************************/
void Usart_SendUserId(USART_TypeDef* pUSARTx, u16 user_id);
void Usart_RFCard_Success(USART_TypeDef* pUSARTx, u16 user_id);
void Usart_RFCard_Error(USART_TypeDef* pUSARTx);
u16 Usart_RecvOrder(USART_TypeDef* pUSARTx);











/********************************************** 底层函数 *************************************************/
void pUsart_SendByte( USART_TypeDef* pUSARTx, u8* ch );
void pUsart_SentMessage( USART_TypeDef* pUSARTx, u16* message);
uint16_t Usart_SendByte( USART_TypeDef* pUSARTx, u8 ch );
uint16_t Usart_RecvByte( USART_TypeDef* pUSARTx);
void debug_usart_init(void);


#endif
