#ifndef _OLED_H_
#define _OLED_H_

#include "stm32f10x.h"
#include "./Delay/delay.h"
#include <stdlib.h>

#define WRITE_MASK	0xFE
#define READ_MASK	0x01


#define OLED_SCL_GPIO_PORT                GPIOB
#define OLED_SCL_GPIO_CLK                 RCC_APB2Periph_GPIOB
#define OLED_SCL_PIN                      GPIO_Pin_3

#define OLED_SDA_GPIO_PORT                GPIOA
#define OLED_SDA_GPIO_CLK                 RCC_APB2Periph_GPIOA
#define OLED_SDA_PIN                      GPIO_Pin_15

void OLED_Init(void);
void OLED_IIC_Init(void);
//void OLED_SetPos(unsigned char x, unsigned char y);
//void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);


void Disp_sentence(uint8_t X,uint8_t Y,char *str,uint8_t clr_or_not);//显示句子
void Disp_sentence_singleline(uint8_t X,uint8_t Y,char *str,uint8_t clr_or_not);
void OLED_Show(unsigned char x, unsigned char y,char data1,char data2,uint8_t CNorEN);//正色单字符
void OLED_Show_inverse(unsigned char x, unsigned char y,char data);//反色单字符


static uint8_t code_111(uint8_t in);
static uint8_t reverse(uint8_t in);
void Disp_line(uint8_t xin,uint8_t yin,uint8_t length,uint8_t hv);
void OLED_Show_Power(uint8_t power);
void show_time_pot(void);
void close_time_pot(void);
int show_time_big(uint8_t xin,uint8_t yin,uint8_t numin);
void show_clock_open_big(void);
void show_hammer(uint8_t xin,uint8_t yin);
#endif
