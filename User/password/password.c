#include "./password/password.h"
#include "./keyboard/keyboard.h"
#include "./data/data_def.h"                    // 为了使用用户结构体
#include "./STMFLASH/stmflash.h"               	// 为了向 flash 中写入射频卡编号
#include "./UT588C/ut588c.h"					// 为了使用喇叭
#include "./SHA_1/sha1.h"

extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];	// 串口接收缓存池

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
	if (length==18)			temp_password_type = MY_PASSWORD_FREE;		// 12字节包头+6字节密码
	else if (length==42)	temp_password_type = MY_PASSWORD_SECTION;	// 12字节包头+6字节密码+2*12字节年月日小时分钟
	else 					temp_password_type = MY_PASSWORD_NURSE;		// 12字节包头+6字节密码+2*4字节小时分钟


	// 如果扫描到空的区域，就直接写入，因为密码不需要考虑“重复”这个问题
	for (u16 i=0; i<500; i++) {
		// 读取数据结构的第一个字节，也就是密码的编号信息，将编号存入临时变量 temp_password_id 中
		addr_now = PASSWORD_ADDR_START +i*MY_PASSWORD_LENGTH;
		STMFLASH_Read(addr_now, &temp_password_id, MY_PASSWORD_LENGTH/2);
		// 如果已经扫描到了空白区域（也就是编号不存在），那就直接写入数据，并且 return ERROR_CODE_SUCCESS
		if ( temp_password_id == 0xFFFF ) {
			*password_id = i;
			temp_password.m_Password_ID = i;
			temp_password.m_Type = temp_password_type;
			temp_password.m_Password = password;
			// 下面开始根据密码的类型来向 flash 写入准入时间以及前面的密码等数据
			switch (temp_password_type){
				// 自由人，不用写准入时间
				case MY_PASSWORD_FREE:
					// 开始写入数据
					STMFLASH_Write(addr_now, (u16*)&temp_password, 4);
					break;

				// 时间段，需要写入所有的准入时间信息
				case MY_PASSWORD_SECTION:
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
				case MY_PASSWORD_NURSE:
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

// 删除密码
// user_number:	需要删除的用户编号
u16 Delete_Password(u16 user_number) {
	u16 no_user = 0xFFFF;

	// 判断用户编号合法性， 如果编号合法，删除用户，return ERROR_CODE_SUCCESS
	if (user_number<500) {
		// 将原有的用户编号置为 0xFFFF，就相当于删除了这个编号下的所有数据
		STMFLASH_Write(PASSWORD_ADDR_START +user_number*MY_PASSWORD_LENGTH, &no_user, 1);
		return ERROR_CODE_SUCCESS;
	}
	// 如果编号非法，return ERROR_CODE_ERROR
	else {
		return ERROR_CODE_ERROR;
	}
}

// 密码验证，判断输入的这个6位密码之前有没有被录入过
// password:	待检验的6位密码，u32格式
// reuturn:		解锁失败或成功
// 现在还没有加入时间段检测！！！！！！！！！！！
u16 Confirm_Password_6Bit(u32 password) {
	u32 addr_now;
	STRUCT_PASSWORD temp_password;

	// 开始从首地址开始查找这个密码有没有被录入过
	for (u16 i=0; i<500; i++) {
		addr_now = PASSWORD_ADDR_START +i*MY_PASSWORD_LENGTH;
		STMFLASH_Read(addr_now, (u16*)&temp_password, 10);

		// 如果这个密码已经录入过了，直接 return ERROR_CODE_SUCCESS。之后需要在这里加入时间段的限制
		if (temp_password.m_Password_ID!=0xFFFF &temp_password.m_Password==password) {
			return ERROR_CODE_SUCCESS;
		}
	}

	// 能跑到这里，说明这张卡并没有被录过，就直接 return ERROR_CODE_ERROR
	return ERROR_CODE_ERROR;
}

// 密码验证，判断输入的这个密码缓存池中包含的所有可能的密码之前有没有被录入过
// buf:			待检验的密码缓存池，u8 Buf[16] 格式
// length:		密码的长度
// reuturn:		解锁失败或成功
// 现在还没有加入时间段检测！！！！！！！！！！！
u16 Confirm_Password(u8* buf, u8 length) {
	u32 password = 0;
	u16 temp_return;

	// 如果密码的长度是6位，那就仅需要转化为u32之后就可以直接进行验证了
	if (length==6) {
		password = buf[0]*100000 +buf[1]*10000 +buf[2]*1000 +buf[3]*100 +buf[4]*10 +buf[5];
		return Confirm_Password_6Bit(password);
	}
	// 如果密码的长度大于6位，就需要将密码缓存池分成多个6位密码然后再化为一个u32格式的整体密码，然后再分别进行验证
	else {
		for (u8 i=0; i<=length-6; i++) {
			password = buf[0+i]*100000 +buf[1+i]*10000 +buf[2+i]*1000 +buf[3+i]*100 +buf[4+i]*10 +buf[5+i];
			temp_return = Confirm_Password_6Bit(password);
			if (temp_return==ERROR_CODE_SUCCESS) {
				return temp_return;
			}
		}

		// 跑到这里说明传统验证方法都已经验证失败，还有可能是用户输入了8位数的随机密码，在判断一次随机密码的准确性
		if (length==8) {
			if( Confirm_Password_SHA1_PasswordAndTime(buf)==ERROR_CODE_SUCCESS || Confirm_Password_SHA1_BLEMacAndTime(buf)==ERROR_CODE_SUCCESS)
				return ERROR_CODE_SUCCESS;
		}

		// 如果能跑到这里，说明这个密码不被通过，就直接 return ERROR_CODE_ERROR
		return ERROR_CODE_ERROR;
	}
}

// 密码验证，判断输入的这个加密的密码对应的真实密码有没有被录入过(密码+时间 作为sha1的输入)
// buf:			待检验的密码缓存池，u8 Buf[8] 格式
// length:		密码的长度
// reuturn:		解锁失败或成功
u16 Confirm_Password_SHA1_PasswordAndTime(u8* buf) {
	u8 sha1_password1[8], sha1_password2[8], sha1_password3[8];
	u32 addr_now;
	STRUCT_PASSWORD temp_password;
	// 生成加密后的密码，每个有效密码生成前后10min共3个加密密码
	for (u16 i=0; i<500; i++) {
		addr_now = PASSWORD_ADDR_START +i*MY_PASSWORD_LENGTH;
		STMFLASH_Read(addr_now, (u16*)&temp_password, 10);
		// 如果这块 flash 区域存储了密码，那就生成3个加密密码，然后与形参进行比较
		if (temp_password.m_Password_ID!=0xFFFF) {
			Pack_PasswordAndTime(temp_password.m_Password, sha1_password1, sha1_password2, sha1_password3);
			if (Compare_2Buf(sha1_password1, buf, 8)==ERROR_CODE_SUCCESS || Compare_2Buf(sha1_password2, buf, 8)==ERROR_CODE_SUCCESS || Compare_2Buf(sha1_password3, buf, 8)==ERROR_CODE_SUCCESS) {
				return ERROR_CODE_SUCCESS;
			}
		}
	}
	// 能跑到这里说明密码验证失败
	return ERROR_CODE_ERROR;
}

// 密码验证，判断输入的这个加密的密码对应的真实密码有没有被录入过(蓝牙mac地址+时间 作为sha1的输入)
// buf:			待检验的密码缓存池，u8 Buf[8] 格式
// length:		密码的长度
// reuturn:		解锁失败或成功
u16 Confirm_Password_SHA1_BLEMacAndTime(u8* buf) {
	u8 sha1_password1[8], sha1_password2[8], sha1_password3[8];

	// 获得当前时间的3个随机8位密码
	Pack_BLEMacAndTime(sha1_password1, sha1_password2, sha1_password3);
	
	// 和输入的8位随机密码进行比较
	if (Compare_2Buf(sha1_password1, buf, 8)==ERROR_CODE_SUCCESS || Compare_2Buf(sha1_password2, buf, 8)==ERROR_CODE_SUCCESS || Compare_2Buf(sha1_password3, buf, 8)==ERROR_CODE_SUCCESS) {
		return ERROR_CODE_SUCCESS;
	}

	else return ERROR_CODE_ERROR;
}

// 新建一个均为【0XFF】的密码缓冲区
void Create_NewPasswordBuf(u8* key_buf) {
	for (u8 i=0; i<LENGTH_KEY_BUF; i++) {
		key_buf[i] = ' ';
	}
}

// 更新按键缓冲区
// key_buf[16]:	准备返回的密码缓存池
// buf_length:	准备返回当前密码缓冲区密码的长度
// last_press:	准备返回最新按下的按键
// return:		当前密码输入的状态
u8 Update_KeyBuf(u8* key_buf, u8* buf_length, u8* last_press) {

	// 捕获按键
	*last_press = Wait_Key();

	// 如果输入超时，就直接 return PASSWORD_CODE_TIMEOUT
	if (*last_press == NO_KEY) {
		return PASSWORD_CODE_TIMEOUT;
	}

	// 如果输入*，表示删除。就清除按键缓存区中的前一个字符
	else if (*last_press == 0x0a) {
		// 如果缓存区中已经没有密码了，说明用户想退出了。return PASSWORD_CODE_QUIT
		if (*buf_length == 0) {
			return PASSWORD_CODE_QUIT;
		}
		// 如果缓存区中还有密码，那就向前删除一位密码。return PASSWORD_CODE_INPUTTING
		else {
			(*buf_length)--;
			key_buf[(*buf_length)-1] = ' ';
			return PASSWORD_CODE_INPUTTING;
		}
	}

	// 如果输入#，表示确认。就开始判断缓存区密码的长度，如果大于等于6位就 return PASSWORD_CODE_COMPLETE
	else if (*last_press == 0x0b){
		if (*buf_length >= 6)	return PASSWORD_CODE_COMPLETE;
		else					return PASSWORD_CODE_SHORT;
	}

	// 如果输入的是数字密码，那就存进缓冲区（如果没有溢出的话）
	else {
		// 如果密码已经存了16个了，没法再加入密码了，就当作没有输入，然后 return PASSWORD_CODE_INPUTTING
		if (*buf_length==LENGTH_KEY_BUF) {
			return PASSWORD_CODE_INPUTTING;
		}
		// 如果密码还能继续存储，那就存吧
		else {
			key_buf[*buf_length] = (*last_press);
			(*buf_length)++;
			return PASSWORD_CODE_INPUTTING;
		}
	}
}
