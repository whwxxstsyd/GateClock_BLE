#ifndef __BLE_H
#define __BLE_H
#include "stm32f10x.h"

// NRST 管脚
#define BLE_NRST_PORT				GPIOA
#define BLE_NRST_PORT_CLK			RCC_APB2Periph_GPIOA
#define BLE_NRST_PIN				GPIO_Pin_11
// PWRC 管脚
#define BLE_PWRC_PORT				GPIOA
#define BLE_PWRC_PORT_CLK			RCC_APB2Periph_GPIOA
#define BLE_PWRC_PIN				GPIO_Pin_12
// INT 管脚
#define BLE_INT_PORT				GPIOB
#define BLE_INT_PORT_CLK			RCC_APB2Periph_GPIOB
#define BLE_INT_PIN					GPIO_Pin_8

#define BLE_INT_EXT_LINE			EXTI_Line8
#define BLE_INT_PIN_SOURCE_PORT		GPIO_PortSourceGPIOB
#define BLE_INT_PIN_SOURCE_PIN		GPIO_PinSource8
#define BLE_INT_EXT_LINE			EXTI_Line8

#define AT_MODE_ON()				BLE_PWRC_LOW()
#define AT_MODE_OFF()				BLE_PWRC_HIGH()
#define BLE_RST_ON()				GPIO_ResetBits(BLE_NRST_PORT,BLE_NRST_PIN)
#define BLE_RST_OFF()				GPIO_SetBits(BLE_NRST_PORT,BLE_NRST_PIN)
#define BLE_PWRC_LOW()				GPIO_ResetBits(BLE_PWRC_PORT,BLE_PWRC_PIN)
#define BLE_PWRC_HIGH()				GPIO_SetBits(BLE_PWRC_PORT,BLE_PWRC_PIN)
#define BLE_INT_EXT_IRQHandler		EXTI9_5_IRQHandler
#define BLE_INT_IRQn				EXTI9_5_IRQn



// BLE 与 手机通信数据包
typedef struct{
	u16 m_magicCode;	// 0xFECF
	u16 m_version;		// 0x0001
	u16 m_totalLength;	// 12
	u16 m_cmdId;		// 单片机主要就看这个信息,来判断这条指令需要单片机做什么
	u16 m_seq;			// None
	u16 m_errorCode;
}BleDataHead;


#define magicCode					0xFECF
#define version						0x0001
#define cmdId_IC					0x0302


#define cmdId_DataToServ			0x0001
// 0xFECF 0001 000E 0001 0000 0000
#define cmdId_DataToServAck			0x1001
#define cmdId_DataToLock			0x0002
#define cmdId_DataToLockAck			0x1002
#define cmdId_AddNotifToLock		0x0003
#define cmdId_AddNotifToLockAck		0x1003
#define cmdId_DelNotifToLock		0x0004
#define cmdId_DelNotifToLockAck		0x1004
#define cmdId_pushToServ			0x2001
#define cmdId_pushToLock			0x2002


#define errorCode0   0x0000   // success
#define errorCode1   0x0001   // 解析数据包失败
#define errorCode2   0x0002   // 进入录入   模式失败
#define errorCode3   0x0003   // 录入模式下超时，cmdId_pushToServ(0x2001)使用
#define errorCode4   0x0004   // 录入模式下两枚指纹不匹配，cmdId_pushToServ(0x2001)使用
#define errorCode5   0x0005   // 指纹删除失败，cmdId_DataToLockAck(0x1002)使用
#define errorCode6   0x0006   // 进入删除模式失败，cmdId_DelNotifToLockAck(0x1004)使用
#define errorCode7   0x0007   // 录完第一次手指后，手机端点击退出录入
#define errorCode8   0x0008   // 进入删除模式，在选择删除指纹界面退出
#define errorCode9   0x0500   // 采集超时






void BLE_init(void);
void BLE_USART_SendStr(char* str);
void BLE_USART_CMD(char* cmd);

#endif
