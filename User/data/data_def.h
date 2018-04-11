#ifndef __DATA_DEF_H
#define __DATA_DEF_H
#include "stm32f10x.h"


// // 用户编号，从1开始，方便用户使用
// // 重要！！
// // flash有限，目前只能存储128个用户，所以在设计用户编号时，只给两位，也就是编号从01~99
// typedef struct//用户准入时间
// {
// 	uint16_t year;
// 	uint8_t month;
// 	uint8_t date;
// 	uint8_t hour;
// 	uint8_t minute;
// 	uint8_t second;
// }USER_TIME;//这个暂时没用 这个是为了以后做每个人开门时间不一样设计的 目前所有保姆用统一的开门时间 下面那个结构体是现在用的

// // 用户准入时间
// typedef struct
// {
// 	// 上限
// 	uint8_t hour;
// 	uint8_t minute;
// 	// 下限
// 	uint8_t hour2;
// 	uint8_t minute2;
// }UNLOCK_TIME;

// // 单个用户历史记录结构体
// typedef struct{
// 	u16 flag;
// 	u16 year;
// 	u16 month_date;
// 	u16 hour_minute;
// 	u16 user_number;
// }UNLOCK_NOTES;

// // 所有用户历史记录结构体
// typedef struct{
// 	UNLOCK_NOTES note1;
// 	UNLOCK_NOTES note2;
// 	UNLOCK_NOTES note3;
// 	UNLOCK_NOTES note4;
// 	UNLOCK_NOTES note5;
// 	UNLOCK_NOTES note6;
// 	UNLOCK_NOTES note7;
// 	UNLOCK_NOTES note8;
// 	UNLOCK_NOTES note9;
// 	UNLOCK_NOTES note10;
// 	UNLOCK_NOTES note11;
// 	UNLOCK_NOTES note12;
// }UNLOCK_NOTES_AREA;



// 用户的数据结构(包括指纹和射频卡)
typedef struct{
	u16 m_USER_Number;	// 编号，【000~999】
	u16 m_USER_Type;	// 类型，【0x0001.指纹】【0x0002.射频卡】
	u32 m_USER_Data;	// 数据，【将m_USER_Type的数据存储在这里】
}MY_USER;

// 密码的数据结构
typedef struct{
	u16 m_Password_ID;		// 编号，【000~999】
	u16 m_Type;				// 密码类型用，来判断准入时间，【0x0001 无时间限制】【0x0002 时间段】【0x0003 保姆】
	u32 m_Password;			// 密码数值
	u16 m_Year_Start;		// 准入开始时间(年)(2018)
	u16 m_MonthAndDay_Start;// 准入开始时间(月日)(1111)
	u16 m_HourAndMin_Start;	// 准入开始时间(时分)(2359)
	u16 m_Year_End;			// 准入结束时间(年)(2018)
	u16 m_MonthAndDay_End;	// 准入结束时间(月日)(1111)
	u16 m_HourAndMin_End;	// 准入结束时间(时分)(2359)
}STRUCT_PASSWORD;

// BLE 与 手机通信数据包
typedef struct{
	u16 m_magicCode;	// 0xFECF
	u16 m_version;		// 0x0001
	u16 m_totalLength;	// 12+
	u16 m_cmdId;		// 单片机主要就看这个信息,来判断这条指令需要单片机做什么
	u16 m_seq;			// None
	u16 m_errorCode;
}BleDataHead;



/********************************************** 用户参数 *************************************************/
#define MY_USER_LENGTH			8			// 一个用户数据结构的大小，通过 sizeof 得到长度位8个字节
#define MY_USER_MAX_NUM			1000		// 最多可以存储120个用户结构体
#define MY_USER_FINGER			0x0001		// 表示这个数据是指纹
#define MY_USER_RFCARD			0x0002		// 表示这个数据是射频卡
#define MY_PASSWORD_LENGTH		20			// 一个密码数据结构的大小
#define MY_PASSWORD_FREE		0x0001		// 表示这个密码没有时间限制
#define MY_PASSWORD_SECTION		0x0002		// 表示这个密码的时间限制为【时间段】
#define MY_PASSWORD_NURSE		0x0003		// 表示这个密码的时间限制位【保姆】


/******************************************** 蓝牙通信参数 ************************************************/
#define USART_RECVBUF_LENGTH	42			// 串口缓存池大小，12包头+6密码+2*12准入时间=12+6+24=42，这个是最大的情况了
#define ERROR_CODE_SUCCESS		0x0000   	// 执行成功
#define ERROR_CODE_TIMEOUT		0x0500   	// 采集超时
#define ERROR_CODE_ERROR		0x0800   	// 异常
#define ERROR_CODE_DUPLICATION	0x0400   	// 信息重复
#define ERROR_CODE_FINGERBAD	0x0300   	// 指纹质量差


/******************************************** 数据包头参数 ************************************************/
#define MAGICCODE				0xFECF
#define VERSION					0x0001


/*********************************************** cmdID ***************************************************/
#define SYS_NO_ORDER			0x3333		// 表示没有接收到有效的包头
#define SYS_RECV_ORDER			0x6666		// 表示接收到了有效的包头
#define CMDID_ADD_FINGER		0x0301		// 添加指纹命令
#define CMDID_DEL_FINGER		0x0802		// 添加指纹命令
#define CMDID_ADD_RFCARD		0x0302		// 添加射频卡命令
#define CMDID_DEL_RFCARD		0x0803		// 删除射频卡命令
#define CMDID_ADD_PASSWORD		0x0104		// 添加密码命令
#define CMDID_DEL_PASSWORD		0x0804		// 删除密码命令


/********************************************** 存储地址 *************************************************/
#define PASSWORD_ADDR_START		0x080092BE	// 存储手机端发来的密码的编号的首地址
#define MY_USER_ADDR_START		0x0800E0BE	// 用户结构体存储首地址
#define MY_USER_ADDR_END		0x0800FFFF	// STM32 Flash 末地址




// 修改这些 记得在初始化函数加上数量初始化
#define alluser_amount_addr		0x800fc00	// 保存所有用户个数
#define admin_amount_addr 		0x800fc02	// 保存管理员个数
#define family_amount_addr 		0x800fc04	// 家人用户
#define babysitter_amount_addr 	0x800fc06	// 保姆用户
#define speak_flag_addr 		0x800fc08	// 音频flag
#define UNLOCK_TIME_addr 		0x800fd00	// 这个结构体存在这个地址
#define UNLOCK_TIME_length 		0x04		// 结构体长度
// 写flash 以2B为单位





void Userid2Ascii(u16 userid, u8* userid_ascii);
u16 Ascii2Userid(u8* userid_ascii);
u16 RecvBuf2Cmdid(void);
u16 RecvBuf2Userid(void);
void RecvBuf2TimeUnlock_SECTION(u16* result);

#endif


// /********************************************* 历史记录参数 **********************************************/
// #define UNLOCK_NOTES_ADDR		0x800fd04	// 用户开锁记录存储首地址
// #define UNLOCK_NOTES_USED_FLAG  0x6666		// 【已经占用】flag，如果开头的16位数据为 UNLOCK_NOTES_USED_FLAG，那么就表明接下来的数据已经被占用了
// #define UNLOCK_NOTES_MSG_LENGTH 5			// 一条信息的总2字节数量，一条信息一共需要写入 USED_FLAG year month_date hour_minute user_number  一共 5*16=80bit 10字节数据，因为每次写都是16bit一写，所以就是5个16bit，5个2字节
// #define UNLOCK_NOTES_SIZE		12			// 能够存储的历史信息的总数量
