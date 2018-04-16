#include "./data/data_def.h"

extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern char BLE_MAC[11];

// 三位数用户编号 转 ascii
// userid:      需要进行转换的用户id号
// userid_ascii:转换完保存到这个地址
void Userid2Ascii(u16 userid, u8* userid_ascii) {
	// 将 userid 分割成三个数字
	u8 bai = userid/100;
	u8 shi = (userid/10)%10;
	u8 ge  = userid%10;

	// 将三个数字转化为 ascii 码
	*userid_ascii = bai+0x30;
	*(userid_ascii+1) = shi+0x30;
	*(userid_ascii+2) = ge+0x30;

	/* 调用示例，串口将会发出【0x313233】
	u8 n[3];
	Userid2Ascii(123,n);
	for (u8 i=0; i<3; i++){
		pUsart_SendByte(UART_1, n+i);
	}*/
}

// ascii 转 三位数用户编号
// userid_ascii:    需要转换的 ascii 数值（24bit）
u16 Ascii2Userid(u8* userid_ascii) {
	u8 bai = (userid_ascii[0])-0x30;
	u8 shi = (userid_ascii[1])-0x30;
	u8 ge  = (userid_ascii[2])-0x30;

	return (bai*100 +shi*10 +ge);
}

// 解析手机端发来的包头数据，返回cmdId
u16 RecvBuf2Cmdid(void) {
	u16 result;
	// 生成16位cmdID
	result = USART_RecvBuf[6];
	result = result<<8;
	result = result | USART_RecvBuf[7];
	return result;
}

// 解析手机端发来的包头数据，返回十进制用户编号
u16 RecvBuf2Userid(void) {
	u16 result;
	u8 userid[3];

	// 抽取 userid
	userid[0] = USART_RecvBuf[12];
	userid[1] = USART_RecvBuf[13];
	userid[2] = USART_RecvBuf[14];

	// 生成十进制数
	result = Ascii2Userid(userid);
	return result;
}

// 解析手机端发来的包头数据，返回准入时间信息（包含年月日小时分钟的格式）
void RecvBuf2TimeUnlock_SECTION(u16* result) {
	u8 qian,bai,shi,ge,num;
	u16 temp;

	// 获取准入时间信息
	for (u8 i=18; i<USART_RECVBUF_LENGTH; i+=4) {
		qian = USART_RecvBuf[i];
		bai = USART_RecvBuf[i+1];
		shi = USART_RecvBuf[i+2];
		ge = USART_RecvBuf[i+3];
		temp = (qian-0x30)*1000 +(bai-0x30)*100 +(shi-0x30)*10 +(ge-0x30);
		num = (i-18)/4;
		result[num] = temp;
	}
}
