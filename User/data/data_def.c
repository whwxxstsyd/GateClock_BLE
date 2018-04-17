#include "./data/data_def.h"
#include "./SHA_1/sha1.h"
#include "./rtc/rtc.h"

extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 BLE_MAC[17];

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

// 将【用户的6位数十进制密码】和【不要分钟的时间信息】连接成一个包，输入至SHA-1，输出三个8位数的随机密码
// password:	十进制的6位数密码
// result1:		前10分钟的（时间+真实密码）生成的 SHA1 随机密码
// result2:		当前时间的（时间+真实密码）生成的 SHA1 随机密码
// result3:		后10分钟的（时间+真实密码）生成的 SHA1 随机密码
void Pack_PasswordAndTime(u32 password, u8* result1, u8* result2, u8* result3) {
	static u16 daycnt=0;
	char boxPassword[17];
	int newPassword;
	u16 temp1=0;
	u32 timecount=0, temp=0;
	u8 hour,min,month,date;
	u16 year;


	// 将十进制数密码拆分为字符串[6]
	boxPassword[0] = password/100000 +0x30;
	boxPassword[1] = (password/10000)%10 +0x30;
	boxPassword[2] = (password/1000)%10 +0x30;
	boxPassword[3] = (password/100)%10 +0x30;
	boxPassword[4] = (password/10)%10 +0x30;
	boxPassword[5] = password%10 +0x30;


	// 获得当前的真正时间戳
    timecount = RTC_GetCounter();


	/********************************** 生成当前时间的 SHA1 加密密码 **********************************/
 	temp = timecount/86400;   //得到天数(秒钟数对应的)
	// 超过一天了
	if (daycnt!=temp) {
		daycnt=temp;
		temp1=1970;	// 从1970年开始
		while (temp>=365) {
			// 是闰年
			if (Is_Leap_Year(temp1)) {
				// 闰年的秒钟数
				if (temp>=366)	temp -= 366;
				else {
					temp1++;
					break;
				}
			}
			// 平年
			else	temp-=365;
			temp1++;
		}
		year=temp1;	// 得到年份
		temp1=0;
		// 超过了一个月
		while(temp>=28) {
			// 当年是不是闰年/2月份
			if(Is_Leap_Year(year) && temp1==1) {
				// 闰年的秒钟数
				if(temp>=29)	temp-=29;//闰年的秒钟数
				else 			break;
			}
			else {
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];	// 平年
				else break;
			}
			temp1++;
		}
		month=temp1+1;	// 得到月份
		date=temp+1;  	// 得到日期
	}
	temp=timecount%86400;     		//得到秒钟数
	hour = temp/3600;     	//小时
	min = (temp%3600)/60; 	//分钟
	// 将十进制的时间拆分为字符串 2018041623 3
	boxPassword[6]	= year/1000 +0x30;
	boxPassword[7]	= year/100%10 +0x30;
	boxPassword[8]	= year/10%10 +0x30;
	boxPassword[9]	= year%10 +0x30;
	boxPassword[10] = month/10 +0x30;
	boxPassword[11] = month%10 +0x30;
	boxPassword[12] = date/10 +0x30;
	boxPassword[13] = date%10 +0x30;
	boxPassword[14] = hour/10 +0x30;
	boxPassword[15] = hour%10 +0x30;
	boxPassword[16] = min/10 +0x30;
	newPassword = sha1(boxPassword, 17);
	// 将生成的 8 位十进制密码转化为字符串形式 u8 result[8];
	Int2Char8(newPassword, result2);


	/********************************** 生成前10分钟的 SHA1 加密密码 **********************************/
	timecount -= 600;
 	temp = timecount/86400;   //得到天数(秒钟数对应的)
	// 超过一天了
	if (daycnt!=temp) {
		daycnt=temp;
		temp1=1970;	// 从1970年开始
		while (temp>=365) {
			// 是闰年
			if (Is_Leap_Year(temp1)) {
				// 闰年的秒钟数
				if (temp>=366)	temp -= 366;
				else {
					temp1++;
					break;
				}
			}
			// 平年
			else	temp-=365;
			temp1++;
		}
		year=temp1;	// 得到年份
		temp1=0;
		// 超过了一个月
		while(temp>=28) {
			// 当年是不是闰年/2月份
			if(Is_Leap_Year(year) && temp1==1) {
				// 闰年的秒钟数
				if(temp>=29)	temp-=29;//闰年的秒钟数
				else 			break;
			}
			else {
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];	// 平年
				else break;
			}
			temp1++;
		}
		month=temp1+1;	// 得到月份
		date=temp+1;  	// 得到日期
	}
	temp=timecount%86400;     		//得到秒钟数
	hour = temp/3600;     	//小时
	min = (temp%3600)/60; 	//分钟
	// 将十进制的时间拆分为字符串 2018041623 3
	boxPassword[6]	= year/1000 +0x30;
	boxPassword[7]	= year/100%10 +0x30;
	boxPassword[8]	= year/10%10 +0x30;
	boxPassword[9]	= year%10 +0x30;
	boxPassword[10] = month/10 +0x30;
	boxPassword[11] = month%10 +0x30;
	boxPassword[12] = date/10 +0x30;
	boxPassword[13] = date%10 +0x30;
	boxPassword[14] = hour/10 +0x30;
	boxPassword[15] = hour%10 +0x30;
	boxPassword[16] = min/10 +0x30;
	newPassword = sha1(boxPassword, 17);
	// 将生成的 8 位十进制密码转化为字符串形式 u8 result[8];
	Int2Char8(newPassword, result1);


	/********************************** 生成后10分钟的 SHA1 加密密码 **********************************/
	timecount += 1200;
 	temp = timecount/86400;   //得到天数(秒钟数对应的)
	// 超过一天了
	if (daycnt!=temp) {
		daycnt=temp;
		temp1=1970;	// 从1970年开始
		while (temp>=365) {
			// 是闰年
			if (Is_Leap_Year(temp1)) {
				// 闰年的秒钟数
				if (temp>=366)	temp -= 366;
				else {
					temp1++;
					break;
				}
			}
			// 平年
			else	temp-=365;
			temp1++;
		}
		year=temp1;	// 得到年份
		temp1=0;
		// 超过了一个月
		while(temp>=28) {
			// 当年是不是闰年/2月份
			if(Is_Leap_Year(year) && temp1==1) {
				// 闰年的秒钟数
				if(temp>=29)	temp-=29;//闰年的秒钟数
				else 			break;
			}
			else {
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];	// 平年
				else break;
			}
			temp1++;
		}
		month=temp1+1;	// 得到月份
		date=temp+1;  	// 得到日期
	}
	temp=timecount%86400;     		//得到秒钟数
	hour = temp/3600;     	//小时
	min = (temp%3600)/60; 	//分钟
	// 将十进制的时间拆分为字符串 2018041623 3
	boxPassword[6]	= year/1000 +0x30;
	boxPassword[7]	= year/100%10 +0x30;
	boxPassword[8]	= year/10%10 +0x30;
	boxPassword[9]	= year%10 +0x30;
	boxPassword[10] = month/10 +0x30;
	boxPassword[11] = month%10 +0x30;
	boxPassword[12] = date/10 +0x30;
	boxPassword[13] = date%10 +0x30;
	boxPassword[14] = hour/10 +0x30;
	boxPassword[15] = hour%10 +0x30;
	boxPassword[16] = min/10 +0x30;
	newPassword = sha1(boxPassword, 17);
	// 将生成的 8 位十进制密码转化为字符串形式 u8 result[8];
	Int2Char8(newPassword, result3);
}

// 将【蓝牙模块的mac地址】和【不要分钟的时间信息】连接成一个包，输入至SHA-1，输出三个8位数的随机密码
// result1:		前10分钟的（时间+真实密码）生成的 SHA1 随机密码
// result2:		当前时间的（时间+真实密码）生成的 SHA1 随机密码
// result3:		后10分钟的（时间+真实密码）生成的 SHA1 随机密码
void Pack_BLEMacAndTime(u8* result1, u8* result2, u8* result3) {
	static u16 daycnt=0;
	char boxPassword[28];
	int newPassword;
	u16 temp1=0;
	u32 timecount=0, temp=0;
	u8 hour,min,month,date;
	u16 year;

	// 载入蓝牙mac地址
	for (u8 i=0; i<17; i++)	boxPassword[i] = BLE_MAC[i];

	// 获得当前的真正时间戳
    timecount = RTC_GetCounter();


	/********************************** 生成当前时间的 SHA1 加密密码 **********************************/
 	temp = timecount/86400;   //得到天数(秒钟数对应的)
	// 超过一天了
	if (daycnt!=temp) {
		daycnt=temp;
		temp1=1970;	// 从1970年开始
		while (temp>=365) {
			// 是闰年
			if (Is_Leap_Year(temp1)) {
				// 闰年的秒钟数
				if (temp>=366)	temp -= 366;
				else {
					temp1++;
					break;
				}
			}
			// 平年
			else	temp-=365;
			temp1++;
		}
		year=temp1;	// 得到年份
		temp1=0;
		// 超过了一个月
		while(temp>=28) {
			// 当年是不是闰年/2月份
			if(Is_Leap_Year(year) && temp1==1) {
				// 闰年的秒钟数
				if(temp>=29)	temp-=29;//闰年的秒钟数
				else 			break;
			}
			else {
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];	// 平年
				else break;
			}
			temp1++;
		}
		month=temp1+1;	// 得到月份
		date=temp+1;  	// 得到日期
	}
	temp=timecount%86400;     		//得到秒钟数
	hour = temp/3600;     	//小时
	min = (temp%3600)/60; 	//分钟
	// 将十进制的时间拆分为字符串 2018041623 3
	boxPassword[17]	= year/1000 +0x30;
	boxPassword[18]	= year/100%10 +0x30;
	boxPassword[19]	= year/10%10 +0x30;
	boxPassword[20]	= year%10 +0x30;
	boxPassword[21] = month/10 +0x30;
	boxPassword[22] = month%10 +0x30;
	boxPassword[23] = date/10 +0x30;
	boxPassword[24] = date%10 +0x30;
	boxPassword[25] = hour/10 +0x30;
	boxPassword[26] = hour%10 +0x30;
	boxPassword[27] = min/10 +0x30;
	newPassword = sha1(boxPassword, 28);
	// 将生成的 8 位十进制密码转化为字符串形式 u8 result[8];
	Int2Char8(newPassword, result2);


	/********************************** 生成前10分钟的 SHA1 加密密码 **********************************/
	timecount -= 600;
 	temp = timecount/86400;   //得到天数(秒钟数对应的)
	// 超过一天了
	if (daycnt!=temp) {
		daycnt=temp;
		temp1=1970;	// 从1970年开始
		while (temp>=365) {
			// 是闰年
			if (Is_Leap_Year(temp1)) {
				// 闰年的秒钟数
				if (temp>=366)	temp -= 366;
				else {
					temp1++;
					break;
				}
			}
			// 平年
			else	temp-=365;
			temp1++;
		}
		year=temp1;	// 得到年份
		temp1=0;
		// 超过了一个月
		while(temp>=28) {
			// 当年是不是闰年/2月份
			if(Is_Leap_Year(year) && temp1==1) {
				// 闰年的秒钟数
				if(temp>=29)	temp-=29;//闰年的秒钟数
				else 			break;
			}
			else {
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];	// 平年
				else break;
			}
			temp1++;
		}
		month=temp1+1;	// 得到月份
		date=temp+1;  	// 得到日期
	}
	temp=timecount%86400;     		//得到秒钟数
	hour = temp/3600;     	//小时
	min = (temp%3600)/60; 	//分钟
	// 将十进制的时间拆分为字符串 2018041623 3
	boxPassword[17]	= year/1000 +0x30;
	boxPassword[18]	= year/100%10 +0x30;
	boxPassword[19]	= year/10%10 +0x30;
	boxPassword[20]	= year%10 +0x30;
	boxPassword[21] = month/10 +0x30;
	boxPassword[22] = month%10 +0x30;
	boxPassword[23] = date/10 +0x30;
	boxPassword[24] = date%10 +0x30;
	boxPassword[25] = hour/10 +0x30;
	boxPassword[26] = hour%10 +0x30;
	boxPassword[27] = min/10 +0x30;
	newPassword = sha1(boxPassword, 17);
	// 将生成的 8 位十进制密码转化为字符串形式 u8 result[8];
	Int2Char8(newPassword, result1);


	/********************************** 生成后10分钟的 SHA1 加密密码 **********************************/
	timecount += 1200;
 	temp = timecount/86400;   //得到天数(秒钟数对应的)
	// 超过一天了
	if (daycnt!=temp) {
		daycnt=temp;
		temp1=1970;	// 从1970年开始
		while (temp>=365) {
			// 是闰年
			if (Is_Leap_Year(temp1)) {
				// 闰年的秒钟数
				if (temp>=366)	temp -= 366;
				else {
					temp1++;
					break;
				}
			}
			// 平年
			else	temp-=365;
			temp1++;
		}
		year=temp1;	// 得到年份
		temp1=0;
		// 超过了一个月
		while(temp>=28) {
			// 当年是不是闰年/2月份
			if(Is_Leap_Year(year) && temp1==1) {
				// 闰年的秒钟数
				if(temp>=29)	temp-=29;//闰年的秒钟数
				else 			break;
			}
			else {
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];	// 平年
				else break;
			}
			temp1++;
		}
		month=temp1+1;	// 得到月份
		date=temp+1;  	// 得到日期
	}
	temp=timecount%86400;     		//得到秒钟数
	hour = temp/3600;     	//小时
	min = (temp%3600)/60; 	//分钟
	// 将十进制的时间拆分为字符串 2018041623 3
	boxPassword[17]	= year/1000 +0x30;
	boxPassword[18]	= year/100%10 +0x30;
	boxPassword[19]	= year/10%10 +0x30;
	boxPassword[20]	= year%10 +0x30;
	boxPassword[21] = month/10 +0x30;
	boxPassword[22] = month%10 +0x30;
	boxPassword[23] = date/10 +0x30;
	boxPassword[24] = date%10 +0x30;
	boxPassword[25] = hour/10 +0x30;
	boxPassword[26] = hour%10 +0x30;
	boxPassword[27] = min/10 +0x30;
	newPassword = sha1(boxPassword, 17);
	// 将生成的 8 位十进制密码转化为字符串形式 u8 result[8];
	Int2Char8(newPassword, result3);
}




// 将 8 位十进制密码转化为字符串形式 u8 result[8];
// newPassword:	8位十进制密码
// result:		u8 字符串
void Int2Char8(int newPassword, u8* result) {
	result[0] = newPassword/10000000;
	result[1] = (newPassword/1000000)%10;
	result[2] = (newPassword/100000)%10;
	result[3] = (newPassword/10000)%10;
	result[4] = (newPassword/1000)%10;
	result[5] = (newPassword/100)%10;
	result[6] = (newPassword/10)%10;
	result[7] = newPassword%10;
}

// 比较两个 u8 数组的成员变量是否完全相同
// a:		数组a
// b:		数组b
// len:		数组的长度
// reuturn:	相同 ERROR_CODE_SUCCESS 或者 不相同 ERROR_CODE_ERROR
u16 Compare_2Buf(u8* a, u8* b, u8 len) {
	u8 count = 0;

	for (u8 i=0; i<len; i++) {
		if (a[i] == b[i])  count++;
	}
	if (count==len)	return ERROR_CODE_SUCCESS;
	else			return ERROR_CODE_ERROR;
}
