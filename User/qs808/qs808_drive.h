#ifndef __QS808_DRIVE
#define __QS808_DRIVE
#include "stm32f10x.h"
#include "./qs808/qs808_usart.h"
//#include "./OLED/oled.h"
#include "./usart/debug_usart.h"
#include "./Delay/delay.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include "my_board.h"
#include "stm32f10x_dma.h"
typedef struct//qs808 接收缓冲区结构体原型
{
	enum Trans_state_enum {reset,set} Trans_state;//发送flag，向qs808发送数据后，这个set
	enum Rec_state_enum {idle,busy} Rec_state;//接收flag，开始接收回包后这个busy 
	uint8_t Rec_point;//缓冲区指针
	uint8_t Rec_Buf[30];//缓冲区
}QS808_Rec_Buf_type;

typedef struct
{	uint16_t HEAD;
	uint8_t SID1;//避免和宏定义中的SID重复
	uint8_t DID1;
	uint16_t RCM;
	uint16_t LEN;
	uint16_t RET;
	uint8_t DATA[14];
	uint16_t CKS;
} QS808_Pack_struct; 


//指令中2B的命令需要先发低8位，再发高8位，所以这里的宏定义特意将高低反着写，所以编程序时
//不用再考虑高低位，直接按先高后低的顺序发
#define QS808_HEAD 	0x55AA
#define SID 				0x00
#define DID 				0x00
//命令
#define	CMD_TEST_CONNECTION 0x0100				//连接测试
#define CMD_FINGER_DETECT 0x2100					//检测手指
#define CMD_GET_IMAGE		0x2000            //采集图像
#define CMD_GENERATE		0x6000						//生成特征
#define CMD_MERGE				0x6100						//合成模板用于入库
#define	CMD_STORE_CHAR	0x4000						//模板入库
#define	CMD_SEARCH			0x6300						//搜索指纹库
#define CMD_GET_EMPTY_ID 0x4500						//获取指纹库可用于注册的首个地址
#define CMD_DEL_CHAR 		 0x4400						//删除指定范围的指纹库
#define CMD_GET_ENROLL_COUNT 0x4800       //获取指纹个数
#define CMD_STANDBY 0x0C00   //低功耗

//错误码
#define ERR_SUCCESS								0x00							//没有错误
#define ERR_FP_NOT_DETECTED 			0x28							//采集器上没有手指
#define ERR_INVALID_BUFFER_ID			0x26							//buffer ID 不正确
#define ERR_BAD_QUALITY						0x19							//指纹质量不好
#define	ERR_GEN_COUNT							0x25							//指纹合成个数无效
#define ERR_MERGE_FAIL						0x1A							//templete合成失败
#define ERR_INVALID_TMPL_NO				0x1D							//templete编号无效
#define ERR_DUPLICATION_ID				0x18							//指纹已经注册
#define	ERR_ALL_TMPL_EMPTY				0x14					//范围内没有指纹（验证时）
#define ERR_INVALID_PARAM					0x22							//使用了不正确的参数
#define ERR_TMPL_EMPTY						0x12							//范围内没有指纹（清空时）
#define	ERR_IDENTIFY							0x11							//指纹库中无此手指
//自定义错误类型
#define QS808_DATA_ERR            0xf0
#define QS808_UNKNOW_ERR          0xf1
#define QS808_WAIT_TIMEOUT				0xf2						//等待输入指纹超时
#define ERR_DEL_FAIL							0xf3					//删除失败
#define ERR_NO_FINGER_DETECT			0xf4					//没有指纹
#define ERR_FINGER_DETECT			0xf5					//有指纹
#define ERR_USART_REC_ERR    0xf6
//函数原型
uint16_t QS808_send_byte(uint8_t byte);
uint16_t QS808_send_half_word(uint16_t half_word);
uint8_t QS808_RecByte(void);
void QS808_Init(void);
uint8_t QS808_TEST_CONNECTION(void);
uint8_t QS808_GET_IMAGE(void);
uint8_t QS808_CMD_GENERATE(uint16_t ram_id);
uint8_t QS808_CMD_MERGE(void);
uint8_t QS808_CMD_STORE_CHAR(uint16_t templete_num);
uint8_t QS808_CMD_GET_EMPTY_ID(uint8_t * ID);
uint8_t	QS808_CMD_DEL_ALL(void);
uint8_t	QS808_CMD_SEARCH(uint8_t * index);
uint8_t	QS808_CMD_DEL_NUM(void);
//上面都是指令级的函数，实际使用请用下面的函数
uint8_t QS808_Wait(uint32_t time);//带延时跳出的指纹读取
uint8_t QS808_Login(uint8_t * ID);//录入指纹 采集3幅图像 合成 存储
uint8_t QS808_SEARCH(uint8_t * index);//指纹验证 
uint8_t QS808_CMD_FINGER_DETECT(void);//检测是否有手指
uint8_t	QS808_CMD_DEL_NUM2(uint8_t index);//根据指纹id删除指纹
//void QS808_CMD_FINGER_DETECT_dma(uint32_t memaddr);
uint16_t QS808_CMD_GET_ENROLL_COUNT(void);
uint8_t QS808_STANDBY(void);
void QS808_Reset(void);
uint8_t QS808_Unpack(QS808_Pack_struct * qs808_rec_bag);
void QS808_Detect_Finger(void);
uint8_t QS808_Detect_Finger_Unpack(void);
void QS808_Rec_Buf_refresh(void);
#endif
