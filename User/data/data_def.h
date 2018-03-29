#ifndef __DATA_DEF_H
#define __DATA_DEF_H
#include "stm32f10x.h"
// 用户编号，从1开始，方便用户使用
// 重要！！
// flash有限，目前只能存储128个用户，所以在设计用户编号时，只给两位，也就是编号从01~99
typedef struct//用户准入时间
{
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}USER_TIME;//这个暂时没用 这个是为了以后做每个人开门时间不一样设计的 目前所有保姆用统一的开门时间 下面那个结构体是现在用的

// 用户准入时间
typedef struct
{
	// 上限
	uint8_t hour;
	uint8_t minute;
	// 下限
	uint8_t hour2;
	uint8_t minute2;
}UNLOCK_TIME;


// error 是预留的debug用的
typedef enum user_type
{admin=1,family,babysitter,temporary,error} user_type;

#define user_flag_value 0xAA

// 用户的数据结构
typedef struct
{
	uint8_t	flag;				// 登记标记，flag=0xAA时，表示录入了用户，否则没有录入
	user_type my_user_type;		// 用户类型 管理员 家人 保姆 客人 临时
	uint16_t number;			// 用户编号,3位数
	uint8_t password_length;	// 密码长度
	uint8_t finger_number;		// 指纹编号,从1开始
	uint8_t password[102];		// 密码数组，最长支持29位,这里106是为了使结构体长度为128。当要再加成员时，把密码数组长度缩短即可
	uint32_t rfcard_id;			// 射频卡id
	USER_TIME time_start;		// 准入时间开始 这个暂时没用 这个是为了以后做每个人开门时间不一样设计的 目前所有保姆用统一的开门时间
	USER_TIME time_stop;		// 准入时间结束
}MY_USER;
#define MY_USER_length 128		// 对应到十六进制就是 80w

// 单个用户历史记录结构体
typedef struct{
	u16 flag;
	u16 year;
	u16 month_date;
	u16 hour_minute;
	u16 user_number;
}UNLOCK_NOTES;

// 所有用户历史记录结构体
typedef struct{
	UNLOCK_NOTES note1;
	UNLOCK_NOTES note2;
	UNLOCK_NOTES note3;
	UNLOCK_NOTES note4;
	UNLOCK_NOTES note5;
	UNLOCK_NOTES note6;
	UNLOCK_NOTES note7;
	UNLOCK_NOTES note8;
	UNLOCK_NOTES note9;
	UNLOCK_NOTES note10;
	UNLOCK_NOTES note11;
	UNLOCK_NOTES note12;
}UNLOCK_NOTES_AREA;







// 每页8个用户数据
#define USER_PER_PAGE 8

#define MY_USER_addr_base 0x800c000		// 从第49页开始存用户结构体
#define MY_USER_addr_offset MY_USER_length*USER_PER_PAGE
#define INIT_ADDR 0x800fc10				// 存放一个关键字，每次用户重置，则重置这个关键字，用于标识是否第一次使用
#define INIT_WORD 0xAA55				// INIT_ADDR存放的初始化关键字
#define MY_USER_addr_end 0x800f800		// 最后一个存用户结构体的页，第63页
#define MY_USER_MAX_NUM 120 			// 最多可以存储120个用户结构体

// 修改这些 记得在初始化函数加上数量初始化
#define alluser_amount_addr		0x800fc00	// 保存所有用户个数
#define admin_amount_addr 		0x800fc02	// 保存管理员个数
#define family_amount_addr 		0x800fc04	// 家人用户
#define babysitter_amount_addr 	0x800fc06	// 保姆用户
#define speak_flag_addr 		0x800fc08	// 音频flag
#define UNLOCK_TIME_addr 		0x800fd00	// 这个结构体存在这个地址
#define UNLOCK_TIME_length 		0x04		// 结构体长度
// 写flash 以2B为单位

#define UNLOCK_NOTES_ADDR		0x800fd04	// 用户开锁记录存储首地址
#define UNLOCK_NOTES_USED_FLAG  0x6666		// 【已经占用】flag，如果开头的16位数据为 UNLOCK_NOTES_USED_FLAG，那么就表明接下来的数据已经被占用了
#define UNLOCK_NOTES_MSG_LENGTH 5			// 一条信息的总2字节数量，一条信息一共需要写入 USED_FLAG year month_date hour_minute user_number  一共 5*16=80bit 10字节数据，因为每次写都是16bit一写，所以就是5个16bit，5个2字节
#define UNLOCK_NOTES_SIZE		12			// 能够存储的历史信息的总数量


#endif
