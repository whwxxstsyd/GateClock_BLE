#ifndef __RC522_FUNCTION_H
#define	__RC522_FUNCTION_H


#include "stm32f10x.h"
#include "./Delay/delay.h"


#define          macDummy_Data              0x00
#define   macRC522_DELAY()  delay_us ( 50 )  // 200us  50*2=100us=10khz

#define   Uart_io_Delay()  delay_us(104)  //104us=9.6khz=baudrate=9600
#define   Uart_io_Delay_half() delay_us(50)

u8 SPI_RC522_SendByte ( u8 byte );
u8 SPI_RC522_ReadByte ( void );
void Uart_RC522_SendByte ( u8 byte );
u8 Uart_RC522_ReadByte ( void );
u8 ReadRawRC ( u8 ucAddress );
void WriteRawRC ( u8 ucAddress, u8 ucValue );
void SetBitMask ( u8 ucReg, u8 ucMask );
void ClearBitMask ( u8 ucReg, u8 ucMask );

void PcdAntennaOn ( void );
void PcdAntennaOff ( void );



void             PcdReset                   ( void );                       //复位

void             M500PcdConfigISOType       ( u8 type );                    //工作方式
char             PcdRequest                 ( u8 req_code, u8 * pTagType ); //寻卡
char             PcdAnticoll                ( u8 * pSnr);                   //读卡号
char PcdSelect ( u8 * pSnr ); // 选卡
char PcdAuthState ( u8 ucAuth_mode, u8 ucAddr, u8 * pKey, u8 * pSnr );  //验证卡片密码
char PcdWrite ( u8 ucAddr, u8 * pData ); //写数据到M1卡一块
char PcdRead ( u8 ucAddr, u8 * pData );  //读取M1卡一块数据
char PcdHalt( void );  //命令卡片进入休眠状态
uint8_t IC_test ( uint32_t *RFCARD_ID );
static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode);

#endif /* __RC522_FUNCTION_H */

