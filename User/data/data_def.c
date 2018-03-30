#include "./data/data_def.h"


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
    u8 bai = (*userid_ascii)-0x30;
    u8 shi = (*userid_ascii+1)-0x30;
    u8 ge  = (*userid_ascii+2)-0x30;

    return (bai*100 +shi*10 +ge);
}
