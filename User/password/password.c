#include "./password/password.h"
#include "./data/data_def.h"                    // 为了使用用户结构体
#include "./STMFLASH/stmflash.h"               // 为了向 flash 中写入射频卡编号
#include "./UT588C/ut588c.h"					// 为了使用喇叭

extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];

// 添加密码
// password_id:	存储分配好的十进制密码ID
// length:		数据包的总长,也就是蓝牙接收包中的 m_seq
// password:	手机端发过来的密码内容（已经化为u16格式）
// time:		手机端发过来的准入时间
// return:		添加成功或者失败
u16 Add_Password(u16* password_id) {
	u32 addr_now;
	u16 temp_password_type,temp_password_id;
	u16 temp_time[6];
	STRUCT_PASSWORD temp_password;

	// 获得数据包的长度
	u8 length = USART_RecvBuf[5];

	// 获得数据包中的密码
	u32 password=0;
	password = password + (u32)(USART_RecvBuf[12]-0x30)*100000;
	password = password + (u32)(USART_RecvBuf[13]-0x30)*10000;
	password = password + (u32)(USART_RecvBuf[14]-0x30)*1000;
	password = password + (u32)(USART_RecvBuf[15]-0x30)*100;
	password = password + (u32)(USART_RecvBuf[16]-0x30)*10;
	password = password + (u32)(USART_RecvBuf[17]-0x30);

	

	// 仅仅知道添加密码是不够的,我们还需要知道需要添加什么样子的准入时间
	if (length==18)			temp_password_type = MY_PASSWARD_FREE;		// 12字节包头+6字节密码
	else if (length==42)	temp_password_type = MY_PASSWARD_SECTION;	// 12字节包头+6字节密码+2*12字节年月日小时分钟
	else 					temp_password_type = MY_PASSWARD_NURSE;		// 12字节包头+6字节密码+2*4字节小时分钟

	// 如果扫描到空的区域，就直接写入，因为密码不需要考虑“重复”这个问题
	for (u16 i=0; i<1000; i++) {
		// 读取数据结构的第一个字节，也就是密码的编号信息，将编号存入临时变量 temp_password_id 中
		addr_now = PASSWARD_ADDR_START +i*MY_PASSWARD_LENGTH;
		STMFLASH_Read(addr_now, &temp_password_id, MY_PASSWARD_LENGTH/2);
		// 如果已经扫描到了空白区域（也就是编号不存在），那就直接写入数据，并且 return ERROR_CODE_SUCCESS
		if ( temp_password_id == 0xFFFF ) {
			*password_id = i;
			temp_password.m_Password_ID = i;
			temp_password.m_Type = temp_password_type;
			temp_password.m_Password = password;
			// 下面开始根据密码的类型来向 flash 写入准入时间以及前面的密码等数据
			switch (temp_password_type){
				// 自由人，不用写准入时间
				case MY_PASSWARD_FREE:
					// 开始写入数据
					STMFLASH_Write(addr_now, (u16*)&temp_password, 4);
					break;
					
				// 时间段，需要写入所有的准入时间信息
				case MY_PASSWARD_SECTION:
					// 获取准入时间段
					RecvBuf2TimeUnlock_SECTION(temp_time);
					temp_password.m_Year_Start = temp_time[0];
					temp_password.m_MonthAndDay_Start = temp_time[1];
					temp_password.m_HourAndMin_Start = temp_time[2];
					temp_password.m_Year_End = temp_time[3];
					temp_password.m_MonthAndDay_End = temp_time[4];
					temp_password.m_HourAndMin_End = temp_time[5];
					// 开始写入数据
					STMFLASH_Write(addr_now, (u16*)&temp_password, 10);
					break;
					
				// 保姆，需要写入 09:00 ~ 17:00 这样的时间信息
				case MY_PASSWARD_NURSE:
					// 获取准入时间段，注意，这个时候的m_Year_Start已经不是开始的年了，而是开始的小时和分钟。
					RecvBuf2TimeUnlock_SECTION(temp_time);
					temp_password.m_Year_Start = temp_time[0];
					temp_password.m_MonthAndDay_Start = temp_time[1];
					// 开始写入数据
					STMFLASH_Write(addr_now, (u16*)&temp_password, 6);
					break;

				default:
					break;
			}
			return ERROR_CODE_SUCCESS;
		}
	}
	// 如果能跑到这里，说明密码的数量已经超过了1000条，flash没有空间了，return ERROR_CODE_ERROR
	return ERROR_CODE_ERROR;
}
