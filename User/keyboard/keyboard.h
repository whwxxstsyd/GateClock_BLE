#ifndef __KEYBOARD_H
#define __KEYBOARD_H
#include "stm32f10x.h"
#include "sys.h"

// 键值表
#define KEY_0			7
#define KEY_1			3
#define KEY_2			4
#define KEY_3			9
#define KEY_4			2
#define KEY_5			5
#define KEY_6			8
#define KEY_7			1
#define KEY_8			6
#define KEY_9			10
#define KEY_AST			0
#define KEY_POU			11
#define KEY_NULL		0xff
#define NO_KEY			0xff

// 与键盘有关的返回值
#define KEY_TIMEOUT		0x01
#define KEY_CANCEL  	0x02
#define KEY_CONFIRM		0x03
#define KEY_TOO_LONG 	0x04
#define WAIT_TIME_MS	10000	// 按键等待 5s
#define WAIT_SCAN_MS	50		// 扫描间隔 50ms



u8 KeyScan(void);
u8 Wait_Key(void);

// uint8_t Key_Cap(uint8_t first_key, uint8_t* length,uint8_t disp_or_not);//缓存按键序列
// uint8_t Key_Cap_6bits(uint8_t first_key, uint8_t* length,uint8_t disp_or_not);//缓存按键序列
// uint8_t admin_verify3_key_cap(uint8_t first_key, uint8_t* length);

#endif
