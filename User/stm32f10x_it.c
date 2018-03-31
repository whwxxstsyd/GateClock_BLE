/**
	******************************************************************************
	* @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
	* @author  MCD Application Team
	* @version V3.2.0
	* @date    03/01/2010
	* @brief   Main Interrupt Service Routines.
	*          This file provides template for all exceptions handler and
	*          peripherals interrupt service routine.
	******************************************************************************
	* @copy
	*
	* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
	* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
	* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
	* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
	* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
	* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
	*
	* <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
	*/

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STM32F10x_StdPeriph_Template
	* @{
	*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
	* @brief   This function handles NMI exception.
	* @param  None
	* @retval None
	*/
#include "stm32f10x_it.h"

extern uint8_t WAKEUP_FLAG;
uint8_t WAKEUP_SOURCE = 0;//0表示指纹唤醒，1表示按键唤醒

extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[12];
extern u8 USART1_RecvBuf_Length;

// //qs808中断
// void QS808_INT_EXT_IRQHandler(void)
// {
// 	if(WAKEUP_FLAG)//休眠时唤醒，需要唤醒其他外设
// 	{
// 		SystemInit();
// 		WAKEUP_FLAG=0;
// 		WAKEUP_SOURCE = 0;
// 		VCC_Adc_Init();
// 		OLED_ON();
// 		Power_ctrl_on();
// 		Uart_RC522_SendByte( 0x55 );
// 		RC522_Init();
// 		TSM12_Wakeup();

// 	}
// 	EXTI_ClearITPendingBit(QS808_INT_EXT_LINE);
// }

// //按键中断
// void TSM12_INT_EXT_IRQHandler(void)
// {
// 	if(WAKEUP_FLAG)
// 	{
// 		SystemInit();
// 		WAKEUP_FLAG=0;
// 		WAKEUP_SOURCE = 1;
// 		VCC_Adc_Init();
// 		QS808_Reset();
// 		OLED_ON();
// 		Power_ctrl_on();
// 		Uart_RC522_SendByte( 0x55 );
// 		RC522_Init();
// 		TSM12_Wakeup();

// 	}
// 	EXTI_ClearITPendingBit(TSM12_INT_EXT_LINE);
// }

//蓝牙中断
void BLE_INT_EXT_IRQHandler(void)
{
	EXTI_ClearITPendingBit(BLE_INT_EXT_LINE);
}

void NMI_Handler(void)
{
}

/**
	* @brief  This function handles Hard Fault exception.
	* @param  None
	* @retval None
	*/
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
	* @brief  This function handles Memory Manage exception.
	* @param  None
	* @retval None
	*/
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
	* @brief  This function handles Bus Fault exception.
	* @param  None
	* @retval None
	*/
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
	* @brief  This function handles Usage Fault exception.
	* @param  None
	* @retval None
	*/
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
	* @brief  This function handles SVCall exception.
	* @param  None
	* @retval None
	*/
void SVC_Handler(void)
{
}

/**
	* @brief  This function handles Debug Monitor exception.
	* @param  None
	* @retval None
	*/
void DebugMon_Handler(void)
{
}

/**
	* @brief  This function handles PendSVC exception.
	* @param  None
	* @retval None
	*/
void PendSV_Handler(void)
{
}

/**
	* @brief  This function handles SysTick Handler.
	* @param  None
	* @retval None
	*/

void SysTick_Handler(void)
{
//	TimingDelay_Decrement();
}

//extern uint8_t BLE_FRAME_REC_FLAG;
//extern char BLE_FRAME[100];
//extern int BLE_FRAME_INDEX;

//uint8_t flag1;
uint8_t debug_data_read_flag=0;//数据导出flag 当flag为1，将flash中的数据从串口导出



// 串口1中断函数，将接收到的任意长度的数据存储在 u8 USART_RecvBuf[12] 中
void USART1_IRQHandler(void){
	// 消除编译器 warrning
	u8 clear = clear;

	// 如果接收到了一字节数据
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET) {
		// 存储一字节数据
		USART_RecvBuf[USART1_RecvBuf_Length++] = USART1->DR;
	}

	// 如果接收到了一帧数据
	else if(USART_GetITStatus(USART1,USART_IT_IDLE) != RESET) {
		// 先读取接受数据寄存器 DR
		clear=USART1->DR;

		// 再读取发送数据寄存器 SR，这么做就可以清除帧中断标志位
		clear=USART1->SR;

		// 标记接收到了一帧数据
		USART_Recv_Flag = 1;
	}
}




// extern QS808_Rec_Buf_type QS808_Rec_Buf;
// uint8_t cnt=0;
// void QS808_USART_RX_ISR(void)
// {
// 	uint8_t ucTemp;
// 	cnt++;
// 	if(USART_GetITStatus(QS808_USART,USART_IT_RXNE)!=RESET)
// 	{
// 		ucTemp = USART_ReceiveData( QS808_USART );
// 		if(QS808_Rec_Buf.Rec_state == idle)//收包空闲时收到包头进入处理
// 		{
// 			if(ucTemp == 0xaa)//收到了包头
// 			{
// 				QS808_Rec_Buf.Rec_state = busy;
// 				QS808_Rec_Buf.Rec_Buf[QS808_Rec_Buf.Rec_point]=ucTemp;
// 				QS808_Rec_Buf.Rec_point++;
// 			}
// 		}
// 		else//收包busy时 收别的数据包
// 		{
// 			QS808_Rec_Buf.Rec_Buf[QS808_Rec_Buf.Rec_point]=ucTemp;
// 			QS808_Rec_Buf.Rec_point++;
// 		}
// 	}
// }
