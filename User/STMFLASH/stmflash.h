#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#define STM32_FLASH_SIZE 64             // 所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_BASE 0x08000000     // FLASH起始地址
#define STM32_FLASH_END  0x0800FFFF     // STM32 FLASH 的结束地址


/********************************************** 用户函数 *************************************************/
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		// 从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		// 从指定地址开始读出指定长度的数据


/********************************************** 底层函数 *************************************************/
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);


#endif
