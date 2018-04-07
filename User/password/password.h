#ifndef __PASSWORD_H
#define __PASSWORD_H
#include "stm32f10x.h"


#define LENGTH_KEY_BUF					16		// 按键缓存区大小，因为屏幕宽度为128，密码宽度为8，128/8=16

#define PASSWORD_CODE_COMPLETE			0x00    // 输入密码完成
#define PASSWORD_CODE_SHORT				0x01    // 输入的密码不足6位
#define PASSWORD_CODE_TIMEOUT			0x02    // 密码输入超时
#define PASSWORD_CODE_QUIT				0x03    // 退出密码输入
#define PASSWORD_CODE_INPUTTING			0x04    // 正在输入密码的过程中






u16 Add_Password(u16* password_id);
u16 Delete_Password(u16 user_number);
u16 Confirm_Password_6Bit(u32 password);
u16 Confirm_Password(u8* buf, u8 length);
void Create_NewPasswordBuf(u8* key_buf);
u8 Update_KeyBuf(u8* key_buf, u8* buf_length, u8* last_press);

#endif
