#ifndef __LOCK_CTRL_H
#define __LOCK_CTRL_H
#include "stm32f10x.h"
#include "./data/data_def.h"
#include <stdio.h>
#include "./TSM12/keyboard.h"
#include "./Delay/delay.h"
#include "stm32f10x_conf.h"
#include  "./qs808/qs808_drive.h"
#include "./STMFLASH/stmflash.h"
#include "./OLED/oled.h"
#include "./UT588C/ut588c.h"
#include "./RC522/rc522_function.h"
#include "./RC522/rc522_config.h"
#include <stdlib.h>


#define USER_LOGIN_SUCCESS 0x00
#define USER_LOGIN_TIMEOUT  0x01
#define USER_LOGIN_BAD_FINGER	0x02
#define USER_LOGIN_DUPLICATION_FINGER 0x03
#define USER_BACK 0x04//按*返回
#define VERIFF_SUCCESS 0x05
#define VERIFF_FAIL 0x06
#define USER_CONFIRM 0x07
#define USER_ID_REDUP 0x08
#define LOGIN_ERROR 0x09

void start_interface(void);     // 【界面】：全屏显示时间
uint8_t unlock(uint8_t key,uint8_t source,uint32_t RFCARD_ID);  // 【功能】：开锁
uint8_t num_unlock_interface(uint8_t first_key, uint8_t* length,uint8_t disp_or_not);   // 【界面】：密码开门

void setting_interface(void);   // 【界面】：验证管理员
void setting_interface2 (void); // 【界面】：设置模式
void admin_settings(void);      // 【界面】：管理员设置
void nuser_settings(void);      // 【界面】：普通用户设置
void system_settings(void);     // 【界面】：系统设置
void nurse_time_settings(void);	// 【界面】：保姆时段设置
void data_note(void);           // 【界面】：数据统计及记录


// 【底层函数】
int first_time_init(void);
void time_disp_big(void);        // 全屏显示时间
void user_struct_store(uint16_t user_num,uint16_t * user_struct);
void user_delete(user_type my_user_type,uint16_t user_num);
void key_disp(uint8_t x,uint8_t y,uint8_t key,uint8_t clr);
void time_disp_bottom(void);    // 在屏幕最下面显示当前时间，格式为：2018-3-5 11:18
void time_disp(void);
void back2factory(void);        // 恢复出厂设置
void unlock_notes_write(u32 addr, uint16_t user_number);
void unlock_notes_set_all_ff(void);
void unlock_notes_set_all_flag_ff(void);
void unlock_notes_disp(u8 pos);
uint8_t password_verify_2admin(uint8_t key_length);
uint8_t password_verify_2unlock(uint8_t key_length, user_type *user_type_o);
uint8_t password_verify_only6(uint8_t key_length, user_type *user_type_o, uint8_t *key_ans);
uint8_t password_verify_only6_admin(uint8_t key_lengthm, uint8_t *key_ans);
uint8_t user_login(user_type my_user_type,uint16_t user_num);
uint8_t admin_mode(void);
uint8_t admin_verify(void);     // 验证管理员函数
uint8_t user_id_cap(uint32_t * user_id,user_type * user_type_o);
uint8_t user_modify(uint16_t user_num);
uint8_t time_settings(void);    // 系统时间设置
uint8_t unlock_time_fun(void);  // 开门时段设置，用于保姆用户
uint8_t time_verify(void);      // 判断当前时间是否属于可开锁时段，用于保姆用户
uint8_t start_interface_key_cap(uint8_t first_key, uint8_t* length,uint8_t disp_or_not);

#endif
