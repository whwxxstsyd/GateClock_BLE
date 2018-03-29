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

//qs808中断
void QS808_INT_EXT_IRQHandler(void)
{
	if(WAKEUP_FLAG)//休眠时唤醒，需要唤醒其他外设
	{
		SystemInit();
		WAKEUP_FLAG=0;
		WAKEUP_SOURCE = 0;
		VCC_Adc_Init();
		OLED_ON();
		Power_ctrl_on();
		Uart_RC522_SendByte( 0x55 );
		RC522_Init();
		TSM12_Wakeup();

	}
	EXTI_ClearITPendingBit(QS808_INT_EXT_LINE);
}

//按键中断
void TSM12_INT_EXT_IRQHandler(void)
{
	if(WAKEUP_FLAG)
	{
		SystemInit();
		WAKEUP_FLAG=0;
		WAKEUP_SOURCE = 1;
		VCC_Adc_Init();
		QS808_Reset();
		OLED_ON();
		Power_ctrl_on();
		Uart_RC522_SendByte( 0x55 );
		RC522_Init();
		TSM12_Wakeup();

	}
	EXTI_ClearITPendingBit(TSM12_INT_EXT_LINE);
}

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

// // 串口1中断服务程序
// void USART1_IRQHandler(void)
// {
// 	uint8_t ucTemp;
// 	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
// 	{
// 		USART_ITConfig( USART1,USART_IT_RXNE,DISABLE );
// 		ucTemp = USART_ReceiveData( USART1 );
// 	}
// 	if (ucTemp==0x12)
// 		debug_data_read_flag=1;
// 	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
// }
// 串口1中断函数
void USART1_IRQHandler(void){
	unsigned char RxData;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		// 清空中断标志位
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

		RxData=USART_ReceiveData(USART1);
		// RxData = RxData + 1;
		USART_SendData(USART1,RxData);


		SPEAK_DUDUDU();
	}
}




extern QS808_Rec_Buf_type QS808_Rec_Buf;
uint8_t cnt=0;
void QS808_USART_RX_ISR(void)
{
	uint8_t ucTemp;
	cnt++;
	if(USART_GetITStatus(QS808_USART,USART_IT_RXNE)!=RESET)
	{
		ucTemp = USART_ReceiveData( QS808_USART );
		if(QS808_Rec_Buf.Rec_state == idle)//收包空闲时收到包头进入处理
		{
			if(ucTemp == 0xaa)//收到了包头
			{
				QS808_Rec_Buf.Rec_state = busy;
				QS808_Rec_Buf.Rec_Buf[QS808_Rec_Buf.Rec_point]=ucTemp;
				QS808_Rec_Buf.Rec_point++;
			}
		}
		else//收包busy时 收别的数据包
		{
			QS808_Rec_Buf.Rec_Buf[QS808_Rec_Buf.Rec_point]=ucTemp;
			QS808_Rec_Buf.Rec_point++;
		}
	}
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
