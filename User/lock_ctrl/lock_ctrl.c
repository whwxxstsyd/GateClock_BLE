#include "./lock_ctrl/lock_ctrl.h"
#include <math.h>
#include <stdlib.h>

extern uint8_t Key_Buffer[30];
extern QS808_Rec_Buf_type QS808_Rec_Buf;
extern unsigned char HZ16x16[];
extern unsigned char F8X16[];
extern uint8_t qs808rec[26];
extern uint32_t ms10_cnt;	// 在 timer.c 中有定义，
extern float 	Battery_quantity;
extern uint8_t debug_data_read_flag;
extern uint8_t WAKEUP_SOURCE;
extern _calendar_obj calendar;//时钟结构体
extern uint8_t timenew;

// 0表示打开语音 1表示关闭语音
uint16_t SPEAK_flag=0;


/******************************************* 时间界面 *******************************************/
// 【界面】：全屏显示时间
void start_interface(){
	uint8_t key;
	uint8_t temp=0;
	uint8_t flag=0;	//标定星号
	uint32_t RFCARD_ID;
	uint32_t standby_cnt=0;
	ms10_cnt=0;
	int qs808_refresh_cnt=30;	//qs808发送看门狗，当这个计数器减为0，说明有1.5s左右没有扫描指纹了，这时应该刷新qs808发送

	QS808_Rec_Buf_refresh();
	Battery_quantity = Get_Battery();

	while(1) {

		// 这个在正式程序中应该被加入，用来增强系统稳定性，但是一般情况下不会进入这个部分
		qs808_refresh_cnt--;
		if(debug_data_read_flag){
			flashdata2usart();
		}
		if(qs808_refresh_cnt<=0){
			qs808_refresh_cnt=30;
			QS808_Rec_Buf_refresh();
		}

		standby_cnt++;

		// 睡眠之前要确保外设都不在工作
		if((standby_cnt >= 100)&&(QS808_Rec_Buf.Trans_state == reset)){
			flag=0;//睡眠之前各种变量清0
			standby_cnt=0;
			QS808_Rec_Buf_refresh();
			// 睡眠之前测一次电量
			Battery_quantity=Get_Battery();
			PWR_Standby_Mode();
			// 唤醒后，从这里还是跑程序
			// 用指纹唤醒系统，跑一次解锁，这样速度快
			if(!WAKEUP_SOURCE){
				Disp_sentence(28,2,"正在验证",1);
				// 指纹解锁,第一个参数随便给
				if( unlock(0x00,1,0x00000000) ==  VERIFF_SUCCESS ){
					standby_cnt = 100;
				}
				// OLED_CLS();
			}
		}

		// 全屏显示时间
		if (standby_cnt<100){
			time_disp_big();
		}

		// 检测到射频卡
		if (IC_test (&RFCARD_ID) == RFCARD_DETECED){
			standby_cnt = 0;
			unlock(0x00,2,RFCARD_ID);//射频卡解锁,第一个参数随便给
		}

		// 检测到指纹
		if (QS808_Rec_Buf.Trans_state == reset){
			qs808_refresh_cnt = 30;
			QS808_Detect_Finger();
		}
		// 这说明收完或者正在收一帧
		if(QS808_Rec_Buf.Rec_state == busy){
			qs808_refresh_cnt = 30;
			temp = QS808_Detect_Finger_Unpack();//解析这一帧
			if(temp == ERR_FINGER_DETECT)
			{
				standby_cnt = 0;
				unlock(0x00,1,0x00000000);//指纹解锁,第一个参数随便给
				QS808_Rec_Buf_refresh();//回到主界面，刷新指纹接收buf
			}
		}

		// 扫描按键
		key = IsKey();
		// *
		if (key==0x0a){
			standby_cnt = 0;
			flag = 1;
		}
		// #
		else if (key == 0x0b){
			standby_cnt = 0;
			if (flag){
				setting_interface();
				QS808_Rec_Buf_refresh();//回到主界面，刷新指纹接收buf
			}
		}
		else if (key != NO_KEY){
			standby_cnt = 0;
			flag = 0;
			// 密码解锁
			unlock(key,0,0x00000000);
			// unlock_interface(key);  这个函数最终函数写在了上面的 unlock 函数中
		}


	}
}


/******************************************* 开锁版块 *******************************************/
// 【功能】：开锁
// key		表示按下的按键
// source	表示解锁源，0对应键盘 1对应指纹 2对应射频
// RFCARD_ID 是rf卡的id 其他开锁模式与这个无关
uint8_t unlock(uint8_t key,uint8_t source,uint32_t RFCARD_ID){
	uint8_t temp_u8;
	uint8_t fingerID = 0x00;
	uint8_t key_length;
	uint32_t addr;
	int i;
	MY_USER my_user1;
	// 初始化一个错误的用户类型，有助于后面的程序debug
	user_type usertype_temp = error;
	// 指纹解锁
	if (source == 1){
		// 在qs808中搜索指纹
		temp_u8 = QS808_SEARCH(&fingerID);
		// 如果搜索到指纹
		if (temp_u8 == ERR_SUCCESS){
			// 根据fingerID判断用户类型
			for (i=0; i<MY_USER_MAX_NUM; i++){
				// 获取
				addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
				// 注意第三个参数是半字数，这很关键，这里写错，导致my_user1越界,所以不论my_user1用局部变量还是malloc都是不对的。
				STMFLASH_Read(addr, (uint16_t*)&my_user1, MY_USER_length/2);
				// 判断是否有有效数据
				if (my_user1.flag == 0xAA){
					if (my_user1.finger_number == fingerID){
						// 缓存用户类型
						usertype_temp = my_user1.my_user_type;
						break;
					}
				}
			}
			// 出bug了,qs808中有指纹，stm32中没有用户，就播报，然后进入【死循环】
			if (usertype_temp == error){
				Disp_sentence(0,0,"error1",1);
				while(1)
				{}
			}
			// 如果是保姆，就还要进行时间验证
			if (usertype_temp == babysitter){
				// 如果不在保姆可行时间段，就播报，清屏，return VERIFF_FAIL
				if(!time_verify()){
					Disp_sentence(32,2,"时间错误",1);
					if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
			}
			// 显示开门界面
			show_clock_open_big();
			// 开锁
			Gate_Unlock();
			// 向 flash 写入历史数据
			unlock_notes_write(UNLOCK_NOTES_ADDR, my_user1.number);
			// 清屏
			OLED_CLS();
			return VERIFF_SUCCESS;
		}
		// 如果没有搜索到指纹，就播报，清屏，return VERIFF_FAIL
		else {
			Disp_sentence(32,2,"验证失败",1);
			if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
			delay_ms(1000);
			OLED_CLS();
			return VERIFF_FAIL;
		}
	}
	// 密码解锁
	else if (source == 0){
		// 进入按键捕捉，记录密码
		// temp_u8 = Key_Cap(key,&key_length,1);
		temp_u8 = num_unlock_interface(key, &key_length,1);

		// 如果按下 * ，就清屏，return USER_BACK
		if(temp_u8 == KEY_CANCEL){
			OLED_CLS();
			return USER_BACK;
		}
		// 如果超时，就播报，清屏，return USER_LOGIN_TIMEOUT
		else if (temp_u8 == KEY_TIMEOUT){
			Disp_sentence(48,2,"超时",1);
			delay_ms(1000);
			OLED_CLS();
			return USER_LOGIN_TIMEOUT;
		}
		// 如果密码过长，就播报，清屏，return USER_BACK
		else if (temp_u8 == KEY_TOO_LONG){
			Disp_sentence(0,2,"密码太长",1);
			delay_ms(1000);
			OLED_CLS();
			return USER_BACK;
		}
		// 如果密码输入完成
		else if (temp_u8 == KEY_CONFIRM){
			// 密码验证函数
			temp_u8 = password_verify_2unlock(key_length, &usertype_temp);
			// 如果是保姆，就再进行一层时间验证
			if (usertype_temp == babysitter){
				// 如果现在不是保姆可行时间，就播报，清屏，return VERIFF_FAIL
				if(!time_verify()){
					Disp_sentence(32,2,"时间错误",1);
					if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
			}
			// 如果验证失败，就播报，清屏，return VERIFF_FAIL
			if (temp_u8 == VERIFF_FAIL){
				Disp_sentence(32,2,"验证失败",1);
				if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
				delay_ms(1000);
				OLED_CLS();
				return VERIFF_FAIL;;
			}
			// 如果验证成功，就播报，开锁，清屏，return VERIFF_SUCCESS
			else{
				// 显示开门界面
				show_clock_open_big();
				// 开锁
				Gate_Unlock();
				// // 向 flash 写入历史数据
				// unlock_notes_write(UNLOCK_NOTES_ADDR, my_user1.number);
				// 清屏
				OLED_CLS();
				return VERIFF_SUCCESS;
			}
		}
	}
	// 射频卡开锁
	else if (source == 2){
		for (i=0;i<MY_USER_MAX_NUM;i++){
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr, (uint16_t*)&my_user1, MY_USER_length/2);
			// 如果这是已经录入过的用户
			if (my_user1.flag == 0xAA){
				// 验证到了射频卡
				if (my_user1.rfcard_id ==  RFCARD_ID){
					// 如果是保姆，就在进行时间验证
					if (my_user1.my_user_type == babysitter){
						// 如果时间错误，就播报，清屏，return VERIFF_FAIL
						if(!time_verify()){
							Disp_sentence(32,2,"时间错误",1);
							if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return VERIFF_FAIL;
						}
						// 如果时间正确，就播报，开锁，清屏，return VERIFF_SUCCESS
						else {
							show_clock_open_big();
							Gate_Unlock();
							OLED_CLS();
							return VERIFF_SUCCESS;
						}
					}
					// 如果不是保姆，就直接播报，开锁，清屏，return VERIFF_SUCCESS
					else {
						show_clock_open_big();
						// 开锁函数
						Gate_Unlock();
						OLED_CLS();
						return VERIFF_SUCCESS;
					}
				}
			}
		}
		// 运行到这里 说明此卡没有录入过
		Disp_sentence(32,2,"验证失败",1);
		if(!SPEAK_flag) SPEAK_OPEN_THE_DOOR_FAIL();
		delay_ms(1000);
		OLED_CLS();
		return VERIFF_FAIL;
	}
	// 如果能运行到这里，就说明验证失败，return VERIFF_FAIL
	return VERIFF_FAIL;
}
// 【界面】：密码开门
uint8_t num_unlock_interface(uint8_t first_key, uint8_t* length,uint8_t disp_or_not){
	uint8_t key;
	int i,j,k;
	*length = 0;
	char disp[9] = "        ";

	OLED_CLS();

	// 【静态】显示第一栏的电池图标，调用了全局变量 Battery_quantity
	if (Battery_quantity<3.7)		OLED_Show_Power(0);
	else if (Battery_quantity<4.2)	OLED_Show_Power(1);
	else if (Battery_quantity<4.7)	OLED_Show_Power(2);
	else if (Battery_quantity<5.2)	OLED_Show_Power(3);
	else 							OLED_Show_Power(4);

	// 【静态显示】第二栏的 ‘请输入密码’
	Disp_sentence_singleline(24,2,"请输入密码",0);

	// 【静态显示】第四栏的当前时间，格式为：2018-3-4 15:16
	time_disp_bottom();



	// 【动态显示】第三栏的密码输入情况，并且在后台持续更新指纹录入情况
	disp[0] = first_key + 48;
	for (i=0;i<30;i++){
		Key_Buffer[i] = NO_KEY;
	}
	Key_Buffer[0] = first_key;
	// 先显示第一个按下的密码符号，需要用到前面捕捉进来的数字 first_key
	Disp_sentence_singleline(0,4,"*",0);

	// 这里i确实是从1开始，而不是0，没有问题,当i等于1时，Key_Buffer只保存了Key_Buffer[0],也就是说，此时只有一位密码
	for (i=1;i<30;i++){
		key = Wait_Key();
		// 输入超时
		if(key == NO_KEY){
			// 输入超时则清空按键buffer
			for (i=0;i<30;i++){
				if(Key_Buffer[i] == NO_KEY)
					break;
				else
					Key_Buffer[i] = NO_KEY;
			}
			return KEY_TIMEOUT;
		}
		// 输入*取消
		else if(key == 0x0a){
			if(i==0){
				return KEY_CANCEL;
			}
			else {
				i=i-1;
				Key_Buffer[i] = NO_KEY;//删除上一位
				i=i-1;//i再-1 抵消for里面的i++
				//显示部分
				k = i-7;//从当前密码位向前数8位开始显示
				j = 0;
				while(j<8){
					if (k>=0){
						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
						j++;
					}
					k++;
				}
			}
		}
		// 输入#确认
		else if (key == 0x0b){
			Key_Buffer[i] = 0x0b;//确认位，用于密码检测
			*length = i;
			if(i==0){
				Disp_sentence(16,4,"密码不能为空",1);
				delay_ms(1000);
				OLED_CLS();
				Key_Buffer[i] = NO_KEY;//删除该位
				i=i-1;//抵消i++,重新录入first_key
			}
			else
				return KEY_CONFIRM;
		}
		// 将按键缓存进buffer
		else{
			Key_Buffer[i] = key;
			// 显示部分
			// 从当前密码位向前数8位开始显示
			k = i-7;
			j = 0;
			while(j<8){
				if (k>=0){
					disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
					j++;
				}
				k++;
			}
		}
		Disp_sentence_singleline(0,4,disp,0);
	}
	//如果程序跑到这里，说明输入的按键串太长了
	return KEY_TOO_LONG;
}


/******************************************* 设置版块 *******************************************/
// 【界面】：验证管理员
// func:	需要输入管理员的密码或者指纹或者射频卡，如果核实是管理员，那就可以进入【界面】：设置模式
void setting_interface(void){
	uint8_t temp;
	uint32_t user_id;
	char disp_arr[4];
	uint32_t while_cnt=0;
	user_type user_type_temp;	// 满足一下函数入口，在这里没用
	QS808_Rec_Buf_refresh();	// 打开新界面，刷新指纹接收buf
	while(1){
		delay_ms(WAIT_SCAN_MS);
		while_cnt++;

		// 如果5s超时，那么就清屏，return
		if (while_cnt == WAIT_TIME_MS/WAIT_SCAN_MS){
			OLED_CLS();
			return;
		}
		// 如果管理员个数为0，就进入管理员注册界面
		if ((*(uint16_t*)admin_amount_addr&(0x00ff)) == 0){
			Disp_sentence(0,3,"请录入一个管理员",1);
			if(!SPEAK_flag) SPEAK_NO_ADMIN_NOW();
			delay_ms(1000);

			// 输入编号
			temp = user_id_cap(&user_id,&user_type_temp);
			// 如果超时，那就清屏，return
			if (temp == USER_LOGIN_TIMEOUT){
				OLED_CLS();
				return;
			}
			// 如果选择后退，那就清屏，return
			else if (temp ==USER_BACK){
				OLED_CLS();
				return;
			}
			// 编号重复
			else if (temp == USER_ID_REDUP){
				Disp_sentence(32,0,"编号重复",1);
				Disp_sentence(24,2,"请重新输入",0);
				if(!SPEAK_flag) SPEAK_OPT_FAIL();
				delay_ms(1000);
				continue;
			}

			disp_arr[0] = user_id/100+48;
			disp_arr[1] = user_id/10%10+48;
			disp_arr[2] = user_id%10+48;
			disp_arr[3] = 0x00;
			//输入编号成功,开始录入
			Disp_sentence(0,0,"录入管理员",1);
			Disp_sentence(0,2,"编号",0);
			Disp_sentence(32,2,disp_arr,0);
			Disp_sentence(0,4,"录指纹或输密码",0);
			Disp_sentence(0,6,"或请刷卡",0);
			if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();
			temp = user_login(admin,user_id);//用户录入函数

			// 如果录入指纹超时，那就播报，清屏，return
			if(temp == USER_LOGIN_TIMEOUT){
				Disp_sentence(48,2,"超时",1);
				delay_ms(1000);
				OLED_CLS();
				return;
			}
			// 如果指纹质量不好、密码太长、密码重复等原因导致的失败，那就 continue
			else if (temp == LOGIN_ERROR){
				continue;
			}
			// 如果录入成功，那就播报，清屏，return
			else if(temp == USER_LOGIN_SUCCESS){
				Disp_sentence(32,2,"录入成功",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				OLED_CLS();
				return;
			}
			// 如果选择后退，那就清屏，return
			else if (temp == USER_BACK){
				OLED_CLS();
				return;
			}
		}
		// 如果管理员人数不为0，就进入管理员验证界面
		else {
			Disp_sentence(32,0,"验证管理员",1);
			Disp_sentence(16,2,"(指纹或密码)",0);
			show_hammer(14,0);
			if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();

			// 开始验证管理员
			temp = admin_verify();

			// 如果选择后退，就清屏，return
			if (temp == USER_BACK){
				OLED_CLS();
				return;
			}
			// 如果超时，就清屏，return
			else if (temp == USER_LOGIN_TIMEOUT){
				OLED_CLS();
				return;
			}
			// 如果验证失败，就 continue
			else if (temp == VERIFF_FAIL)
				continue;
			// 如果验证成功，就进入【设置模式第二级】界面
			else if (temp == VERIFF_SUCCESS){
				setting_interface2();
				OLED_CLS();
				return; // 跑到这里，说明已经完成了设置，直接reurn到主界面,别忘了清屏
			}
		}
	}
}
// 【界面】：设置模式
// func:	可以进行4大项的设置（管理员设置、普通用户设置、系统设置、附加设置）
void setting_interface2 (void){
	uint8_t key;
	while(1){
		Disp_sentence(0,0,"1.管理员设置",1);
		Disp_sentence(0,2,"2.普通用户设置",0);
		Disp_sentence(0,4,"3.系统设置",0);
		Disp_sentence(0,6,"4.数据统计及记录",0);

		key = Wait_Key();

		// 如果超时，return
		if (key == NO_KEY){
			// Disp_sentence(48,2,"超时",1);
			return;
		}
		// 如果按下*，return
		else if (key == 0x0a){
			return;
		}
		// 如果按下1，就进入【管理员设置】界面
		else if (key == 0x01){
			admin_settings();
		}
		// 如果按下2，就进入【普通用户设置】界面
		else if (key == 0x02){
			nuser_settings();
		}
		// 如果按下3，就进入【系统设置】界面
		else if (key == 0x03){
			system_settings();
		}
		// 如果按下4，就进入【保姆时段设置】界面
		else if (key == 0x04){
			data_note();
		}
	}
}
// 【界面】：管理员设置
void admin_settings(void){
	uint8_t key;
	uint8_t temp_u8;
	uint32_t user_id;
	char disp_arr[4];
	user_type user_type_temp;//录入时没用，修改和删除时，用过这个判断对应的编号是不是管理员
	uint8_t flag=0;//这个flag标记输入编号时用户按下返回键，根据这个flag，重新回到admin_settings界面
	while(1)
	{
		Disp_sentence(0,0,"1.录入管理员",1);
		Disp_sentence(0,2,"2.修改管理员",0);
		Disp_sentence(0,4,"3.删除管理员",0);
		key = Wait_Key();
		if (key == NO_KEY)//等待按键超时
		{
			Disp_sentence(48,2,"超时",1);
			delay_ms(1000);
			return;
		}
		else if (key == 0x0a)//星号返回
		{
			return;
		}
		/**********************选项1 录入管理员 ******************************/
		else if (key == 0x01)//录入管理员
		{
			while(1)
				{
					while(1)//输入编号
					{
						temp_u8 = user_id_cap(&user_id,&user_type_temp);
						if (temp_u8 == USER_LOGIN_TIMEOUT)
							return;
						else if (temp_u8 ==USER_BACK)
						{
							flag = 1;
							break;
						}
						else if (temp_u8 == USER_ID_REDUP)//编号重复，重新输入编号
						{
							Disp_sentence(25,0,"编号重复",1);
							Disp_sentence(20,2,"请重新输入",0);
							delay_ms(1000);
							continue;
						}
						else //编号合法
							break;
					}
					if (flag)
					{
						flag = 0;
						break;//这break 会使admin_settings函数重跑
					}

					disp_arr[0] = user_id/100+48;
					disp_arr[1] = user_id/10%10+48;
					disp_arr[2] = user_id%10+48;
					disp_arr[3] = 0x00;


				//输入编号成功,开始录入

					Disp_sentence(0,0,"录入管理员",1);
					Disp_sentence(0,2,"编号",0);
					Disp_sentence(32,2,disp_arr,0);
					Disp_sentence(0,4,"录指纹或输密码",0);
				  Disp_sentence(0,6,"或请刷卡",0);
					if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();
					temp_u8 = user_login(admin,user_id);//超时或按星号退出 否则继续录入
					if (temp_u8 == USER_LOGIN_TIMEOUT)
					{
						Disp_sentence(48,2,"超时",1);
						delay_ms(1000);
						return;
					}
					else if (temp_u8 == LOGIN_ERROR)//指纹质量不好，密码太长，密码重复等原因导致的失败
					{
						continue;
					}
					else if (temp_u8 == USER_BACK)
						return;
					else
					{
						Disp_sentence(0,0,"录入成功",1);
						Disp_sentence(0,2,"按#继续录入",0);
						Disp_sentence(0,4,"按*返回",0);
						if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
						key = Wait_Key();
						if (key == NO_KEY)//等待按键超时
						{
							Disp_sentence(48,2,"超时",1);
							return;
						}
						else if (key == 0x0a)//星号返回
						{
							break;//返回当前界面，到管理员设置界面
						}
						else if (key == 0x0b)//井号继续
							continue;
						//其它键 不响应
					}
				}
		}
	/***********************选项2 修改管理员 **********************/
		else if (key == 0x02)//修改管理员
		{
			while(1)
			{
				while(1)//输入编号
				{
					temp_u8 = user_id_cap(&user_id,&user_type_temp);
					if (temp_u8 == USER_LOGIN_TIMEOUT)
						return;
					else if (temp_u8 ==USER_BACK)
					{
						flag = 1;
						break;
					}
					else if (temp_u8 != USER_ID_REDUP)//没有这个编号
					{
						Disp_sentence(0,2,"编号不存在",1);
						Disp_sentence(0,4,"请重新输入",0);
					  if(!SPEAK_flag) SPEAK_OPT_FAIL();
						delay_ms(1000);
						continue;
					}
					else if(user_type_temp != admin)//此编号不是管理员
					{
						Disp_sentence(0,2,"编号不是管理员",1);
						Disp_sentence(0,4,"请重新输入",0);
						if(!SPEAK_flag) SPEAK_OPT_FAIL();
						delay_ms(1000);
						continue;
					}
					else //编号合法
						break;
				}
				if (flag)
				{
					flag = 0;
					break;//这break 会使admin_settings函数重跑
				}



				disp_arr[0] = user_id/100+48;
				disp_arr[1] = user_id/10%10+48;
				disp_arr[2] = user_id%10+48;
				disp_arr[3] = 0x00;


				//输入编号成功,开始修改
				Disp_sentence(0,0,"修改管理员",1);
				Disp_sentence(0,2,"编号",0);
				Disp_sentence(32,2,disp_arr,0);
				Disp_sentence(0,4,"录指纹或输密码",0);
        		Disp_sentence(0,6,"或请刷卡",0);
				if(!SPEAK_flag) SPEAK_CARD_FIGER_OR_PW();
				temp_u8 = user_modify(user_id);
				if(temp_u8 == USER_LOGIN_TIMEOUT)//超时
				{
					Disp_sentence(48,2,"超时",1);
					delay_ms(1000);
					OLED_CLS();
					return;
				}
				else if (temp_u8 == USER_LOGIN_SUCCESS)
				{
						Disp_sentence(0,0,"修改成功",1);
						Disp_sentence(0,2,"按#继续修改",0);
						Disp_sentence(0,4,"按*返回",0);
					  if(!SPEAK_flag) SPEAK_OPT_SUCCESS() ;
						key = Wait_Key();
						if (key == NO_KEY)//等待按键超时
						{
							Disp_sentence(48,2,"超时",1);
							return;
						}
						else if (key == 0x0a)//星号返回
						{
							break;//返回当前界面，到管理员设置界面
						}
						else if (key == 0x0b)//井号继续
							continue;
						//其它键 不响应
				}
				else if (temp_u8 == LOGIN_ERROR)
				{
					continue;
				}
				else
					return;//修改超时，或者主动返回，都直接返回
			}
		}
		/**********************功能3 删除管理员************************/
		else if (key == 0x03)//删除管理员
		{
			while(1)
			{
				Disp_sentence(0,2,"1.按编号删除",1);
				Disp_sentence(0,4,"2.全部删除",0);
				key = Wait_Key();
				if (key == NO_KEY)//等待按键超时
				{
					Disp_sentence(48,2,"超时",1);
					delay_ms(1000);
					return;
				}
				else if (key == 0x0a)//星号返回
				{
					break;
				}
				else if (key == 0x01)//按编号删除
				{

					while(1)//输入编号
					{
						temp_u8 = user_id_cap(&user_id,&user_type_temp);
						if (temp_u8 == USER_LOGIN_TIMEOUT)
							return;
						else if (temp_u8 ==USER_BACK)
						{
							flag = 1;
							break;
						}
						else if (temp_u8 != USER_ID_REDUP)//没有这个编号
						{
							Disp_sentence(0,2,"编号不存在",1);
							Disp_sentence(0,4,"请重新输入",0);
							delay_ms(1000);
							continue;
						}
						else if(user_type_temp != admin)//此编号不是管理员
						{
							Disp_sentence(0,2,"编号不是管理员",1);
							Disp_sentence(0,4,"请重新输入",0);
							delay_ms(1000);
							continue;
						}
						else //编号合法
							break;
					}
						if (flag)
						{
							flag = 0;
							break;
						}
						disp_arr[0] = user_id/100+48;
						disp_arr[1] = user_id/10%10+48;
						disp_arr[2] = user_id%10+48;
						disp_arr[3] = 0x00;
						Disp_sentence(0,0,"删除",1);
						Disp_sentence(32,0,disp_arr,0);
						Disp_sentence(0,2,"按#确认",0);
						Disp_sentence(0,4,"按*返回",0);
						key = Wait_Key();
						if (key == NO_KEY)//等待按键超时
						{
							Disp_sentence(48,2,"超时",1);
							delay_ms(1000);
							return;
						}
						else if (key == 0x0a)//星号返回
						{
							break;
						}
						else if (key == 0x0b)//井号继续
						{
							user_delete(admin,user_id);//删除
							Disp_sentence(0,0,"删除成功",1);
							Disp_sentence(0,2,"按#继续删除",0);
							Disp_sentence(0,4,"按*返回",0);
							if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
							key = Wait_Key();
							if (key == NO_KEY)//等待按键超时
							{
								Disp_sentence(48,2,"超时",1);
								delay_ms(1000);
								return;
							}
							else if (key == 0x0a)//星号返回
							{
								break;//返回当前界面，到管理员设置界面
							}
							else if (key == 0x0b)//井号继续
								continue;
							//其它键 不响应
						}
						//其它键 不响应
				}
				else if (key == 0x02)//全部删除
				{
					Disp_sentence(0,0,"删除全部",1);
					Disp_sentence(0,2,"按#确认",0);
					Disp_sentence(0,4,"按*返回",0);
					key = Wait_Key();
					if (key == NO_KEY)//等待按键超时
					{
						Disp_sentence(48,2,"超时",1);
						delay_ms(1000);
						return;
					}
					else if (key == 0x0a)//星号返回
					{
						break;
					}
					else if (key == 0x0b)//井号继续
					{
						user_delete(admin,0xffff);
						Disp_sentence(25,2,"删除成功",1);
						if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
						delay_ms(1000);
						break;
					}
					//其它键 不响应
				}
			}
		}
	}
}
// 【界面】：普通用户设置
void nuser_settings(void){
	uint8_t key;
	uint8_t temp_u8;
	uint32_t user_id;
	char disp_arr[4];
	user_type user_type_temp;//录入时没用，修改和删除时，用过这个判断对应的编号是不是普通用户
	user_type user_type_temp2;//记录用户想要操作的的类型
	uint8_t flag=0;//这个flag标记输入编号时用户按下返回键，根据这个flag，重新回到admin_settings界面
	while(1)
	{
		Disp_sentence(0,0,"1.录入普通用户",1);
		Disp_sentence(0,2,"2.修改普通用户",0);
		Disp_sentence(0,4,"3.删除普通用户",0);
		key = Wait_Key();

		// 超时，return
		if (key == NO_KEY){
			Disp_sentence(48,2,"超时",1);
			delay_ms(1000);
			return;
		}
		// *，return
		else if (key == 0x0a){
			return;
		}
		// 按下1，进入【录入普通用户】界面
		else if (key == 0x01){
			while(1)
			{
				//选择用户类型
				Disp_sentence(0,0,"选择用户类型",1);
				Disp_sentence(0,2,"1.家人",0);
				Disp_sentence(0,4,"2.保姆",0);
				while(1){
					key = Wait_Key();

					// 等待按键超时
					if (key == NO_KEY){
						Disp_sentence(48,2,"超时",1);
						delay_ms(1000);
						return;
					}
					// 星号返回
					else if (key == 0x0a){
						flag = 1;
						break;
					}
					// 家人
					else if (key == 0x01){
						user_type_temp2 = family;
						break;
					}
					// 保姆
					else if (key == 0x02){
						user_type_temp2 = babysitter;
						break;
					}
					//其它类型等待加入
				}

				// 如果 flag==1 ，那就 break 返回上一层菜单
				if (flag){
					flag = 0;
					break;
				}

				// 输入编号
				while(1){
					temp_u8 = user_id_cap(&user_id,&user_type_temp);
					if (temp_u8 == USER_LOGIN_TIMEOUT){
						return;
					}
					else if (temp_u8 == USER_BACK){
						flag = 1;
						break;
					}
					// 编号重复，重新输入编号
					else if (temp_u8 == USER_ID_REDUP){
						Disp_sentence(0,2,"编号重复",1);
						Disp_sentence(0,4,"请重新输入",0);
						delay_ms(1000);
						continue;
					}
					// 编号合法，break 返回上一层菜单
					else {
						break;
					}
				}

				// 如果 flag==1 ，那就 break 返回上一层菜单
				if (flag){
					flag = 0;
					break;	//返回上一层
				}

				disp_arr[0] = user_id/100+48;
				disp_arr[1] = user_id/10%10+48;
				disp_arr[2] = user_id%10+48;
				disp_arr[3] = 0x00;

				// 输入编号成功,开始录入
				Disp_sentence(0,0,"录入普通用户",1);
				Disp_sentence(0,2,"编号",0);
				Disp_sentence(32,2,disp_arr,0);
				Disp_sentence(0,4,"录指纹或输密码",0);
				Disp_sentence(0,6,"或请刷卡",0);
				temp_u8 = user_login(user_type_temp2,user_id);//超时或按星号退出 否则继续录入
				if (temp_u8 == USER_LOGIN_TIMEOUT){
					Disp_sentence(48,2,"超时",1);
					delay_ms(1000);
					return;
				}
				// 指纹质量不好，密码太长，密码重复等原因导致的失败
				else if (temp_u8 == LOGIN_ERROR){
					continue;
				}
				else if (temp_u8 == USER_BACK){
					return;
				}
				else {
					Disp_sentence(0,0,"录入成功",1);
					Disp_sentence(0,2,"按#继续录入",0);
					Disp_sentence(0,4,"按*返回",0);
					key = Wait_Key();
					if (key == NO_KEY)//等待按键超时
					{
						Disp_sentence(48,2,"超时",1);
						return;
					}
					else if (key == 0x0a)//星号返回
					{
						break;//返回当前界面，到管理员设置界面
					}
					else if (key == 0x0b)//井号继续
						continue;
					//其它键 不响应
				}
			}
		}
		// 按下2，进入【修改普通用户】界面
		else if (key == 0x02){
			while(1){
				//选择用户类型
				Disp_sentence(0,0,"选择用户类型",1);
				Disp_sentence(0,2,"1.家人",0);
				Disp_sentence(0,4,"2.保姆",0);
				while(1){
					key = Wait_Key();
					if (key == NO_KEY)//等待按键超时
					{
						Disp_sentence(48,2,"超时",1);
						delay_ms(1000);
						return;
					}
					else if (key == 0x0a)//星号返回
					{
						flag = 1;
						break;
					}
					else if (key == 0x01)//家人
					{
						user_type_temp2 = family;
						break;
					}
					else if (key == 0x02)//保姆
					{
						user_type_temp2 = babysitter;
						break;
					}
					//其它类型等待加入
				}
				// 返回上一层菜单
				if (flag){
					flag = 0;
					break;
				}
				// 输入编号
				while(1){
					temp_u8 = user_id_cap(&user_id,&user_type_temp);
					if (temp_u8 == USER_LOGIN_TIMEOUT)
						return;
					else if (temp_u8 ==USER_BACK)
					{
						flag = 1;
						break;
					}
					else if (temp_u8 != USER_ID_REDUP)//没有这个编号
					{
						Disp_sentence(0,2,"编号不存在",1);
						Disp_sentence(0,4,"请重新输入",0);
						delay_ms(1000);
						continue;
					}
					else if(user_type_temp != user_type_temp2)//此编号不是要修改的用户类型
					{
						Disp_sentence(0,2,"用户类型错误",1);
						Disp_sentence(0,4,"请重新输入",0);
						delay_ms(1000);
						continue;
					}
					else //编号合法
						break;
				}
				if (flag){
					flag = 0;
					break;//这break 会使admin_settings函数重跑
				}

				disp_arr[0] = user_id/100+48;
				disp_arr[1] = user_id/10%10+48;
				disp_arr[2] = user_id%10+48;
				disp_arr[3] = 0x00;

				//输入编号成功,开始修改
				Disp_sentence(0,0,"修改普通用户",1);
				Disp_sentence(0,2,"编号",0);
				Disp_sentence(32,2,disp_arr,0);
				Disp_sentence(0,4,"录指纹或输密码",0);
        		Disp_sentence(0,6,"或请刷卡",0);

				temp_u8 = user_modify(user_id);
				if(temp_u8 == USER_LOGIN_TIMEOUT)//超时
				{
					Disp_sentence(48,2,"超时",1);
					delay_ms(1000);
					OLED_CLS();
					return;
				}
				else if (temp_u8 == USER_LOGIN_SUCCESS)
				{
						Disp_sentence(0,0,"修改成功",1);
						Disp_sentence(0,2,"按#继续修改",0);
						Disp_sentence(0,4,"按*返回",0);
						key = Wait_Key();
						if (key == NO_KEY)//等待按键超时
						{
							Disp_sentence(48,2,"超时",1);
							return;
						}
						else if (key == 0x0a)//星号返回
						{
							break;//返回当前界面，到管理员设置界面
						}
						else if (key == 0x0b)//井号继续
							continue;
						//其它键 不响应
				}
				else if (temp_u8 == LOGIN_ERROR)
				{
					continue;
				}
				else
					return;//修改超时，或者主动返回，都直接返回
			}
		}
		// 按下3，进入【删除普通用户】界面
		else if (key == 0x03){
			while(1){

				//选择用户类型
				Disp_sentence(0,0,"选择用户类型",1);
				Disp_sentence(0,2,"1.家人",0);
				Disp_sentence(0,4,"2.保姆",0);
				while(1){
					key = Wait_Key();
					// 等待按键超时
					if (key == NO_KEY){
						Disp_sentence(48,2,"超时",1);
						delay_ms(1000);
						return;
					}
					// 按下 * ，将 flag 置一
					else if (key == 0x0a){
						flag = 1;
						break;
					}
					// 家人
					else if (key == 0x01){
						user_type_temp2 = family;
						break;
					}
					// 保姆
					else if (key == 0x02){
						user_type_temp2 = babysitter;
						break;
					}
				}
				// 如果 flag 为1，break
				if (flag){
					flag = 0;
					// 这break 会使admin_settings函数重跑
					break;
				}
				Disp_sentence(0,2,"1.按编号删除",1);
				Disp_sentence(0,4,"2.全部删除",0);
				key = Wait_Key();
				// 等待按键超时
				if (key == NO_KEY){
					Disp_sentence(48,2,"超时",1);
					delay_ms(1000);
					return;
				}
				// 星号返回
				else if (key == 0x0a){
					break;
				}
				// 按编号删除
				else if (key == 0x01){

					while(1)//输入编号
					{
						temp_u8 = user_id_cap(&user_id,&user_type_temp);
						if (temp_u8 == USER_LOGIN_TIMEOUT)
							return;
						else if (temp_u8 ==USER_BACK)
						{
							flag = 1;
							break;
						}
						else if (temp_u8 != USER_ID_REDUP)//没有这个编号
						{
							Disp_sentence(24,0,"编号不存在",1);
							Disp_sentence(24,2,"请重新输入",0);
							delay_ms(1000);
							continue;
						}
						else if(user_type_temp != user_type_temp2)
						{
							Disp_sentence(24,0,"用户类型错误",1);
							Disp_sentence(24,2,"请重新输入",0);
							delay_ms(1000);
							continue;
						}
						else //编号合法
							break;
					}
					if (flag){
						flag = 0;
						break;
					}
					disp_arr[0] = user_id/100+48;
					disp_arr[1] = user_id/10%10+48;
					disp_arr[2] = user_id%10+48;
					disp_arr[3] = 0x00;
					Disp_sentence(0,0,"删除",1);
					Disp_sentence(32,0,disp_arr,0);
					Disp_sentence(0,2,"按#确认",0);
					Disp_sentence(0,4,"按*返回",0);
					key = Wait_Key();
					// 等待按键超时
					if (key == NO_KEY){
						Disp_sentence(48,2,"超时",1);
						delay_ms(1000);
						return;
					}
					// * 返回
					else if (key == 0x0a){
						break;
					}
					// # 继续
					else if (key == 0x0b){
						user_delete(user_type_temp2,user_id);//删除
						Disp_sentence(0,0,"删除成功",1);
						Disp_sentence(0,2,"按#继续删除",0);
						Disp_sentence(0,4,"按*返回",0);
						key = Wait_Key();
						// 等待按键超时
						if (key == NO_KEY){
							Disp_sentence(48,2,"超时",1);
							delay_ms(1000);
							return;
						}
						// 星号返回
						else if (key == 0x0a){
							break;
						}
						// 井号继续
						else if (key == 0x0b)
							continue;
					}
				}
				// 全部删除
				else if (key == 0x02){
					Disp_sentence(0,0,"删除全部",1);
					Disp_sentence(0,2,"按#确认",0);
					Disp_sentence(0,4,"按*返回",0);
					key = Wait_Key();
					if (key == NO_KEY)//等待按键超时
					{
						Disp_sentence(48,2,"超时",1);
						delay_ms(1000);
						return;
					}
					else if (key == 0x0a)//星号返回
					{
						break;
					}
					else if (key == 0x0b)//井号继续
					{
						user_delete(user_type_temp2,0xffff);
						Disp_sentence(32,2,"删除成功",1);
						delay_ms(1000);
						break;
					}
					//其它键 不响应
				}
			}
		}
	}
}
// 【界面】：系统设置
void system_settings(void){
	uint8_t u8_temp;
	while(1){
		Disp_sentence(0,0,"1.系统时间设置",1);
		Disp_sentence(0,2,"2.语音设置",0);
		Disp_sentence(0,4,"3.恢复出厂设置",0);
		Disp_sentence(0,6,"4.保姆时段设置",0);
		uint8_t key;
		key = Wait_Key();

		// 超时，return
		if (key == NO_KEY){
			// Disp_sentence(48,2,"超时",1);
			return;
		}
		// *，return
		else if (key == 0x0a){
			return;
		}
		// 按下1，进入【时间设置】界面
		else if (key == 0x01){
			// 进入【系统时间设置界面】，并返回 temp
			u8_temp = time_settings();
			if (u8_temp == USER_LOGIN_TIMEOUT)
				return;
			else if (u8_temp == USER_CONFIRM)
			{
				Disp_sentence(25,2,"设置成功",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				continue;
			}
		}
		// 按下2，进入【语音设置】界面
		else if (key == 0x02){
			Disp_sentence(0,0,"1.打开语音",1);
			Disp_sentence(0,2,"2.关闭语音",0);
			key = Wait_Key();
			if (key == NO_KEY)//等待按键超时
			{
				Disp_sentence(48,2,"超时",1);
				delay_ms(1000);
				return;
			}
			else if (key == 0x0a)//星号返回
			{
				break;
			}
			else if (key == 0x01)//打开语音
			{
				SPEAK_flag=0;
				STMFLASH_Write(speak_flag_addr,&SPEAK_flag,1);//打开语音
				Disp_sentence(25,2,"设置成功",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				break;
			}
			else if (key == 0x02)//关闭语音
			{
				//待编辑
				SPEAK_flag=1;
				STMFLASH_Write(speak_flag_addr,&SPEAK_flag,1);//打开语音
				Disp_sentence(25,2,"设置成功",1);
				if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
				delay_ms(1000);
				break;
			}
		}
		// 按下3，进入【恢复出厂设置】界面
		else if (key == 0x03){
				Disp_sentence(0,0,"确认恢复出厂设置",1);
				Disp_sentence(0,2,"按#确认",0);
				Disp_sentence(0,4,"按*返回",0);
				key = Wait_Key();
				if (key == NO_KEY)//等待按键超时
				{
					Disp_sentence(48,2,"超时",1);
					delay_ms(1000);
					return;
				}
				else if (key == 0x0a)//星号返回
				{
					break;
				}
				else if (key == 0x0b)//井号继续
				{
					back2factory();//出厂设置
					Disp_sentence(0,2,"恢复出厂设置成功",1);
					if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
					delay_ms(1000);
					NVIC_SystemReset();
				}

		}
		// 按下4，进入【保姆时段设置】
		else if (key == 0x04){
			nurse_time_settings();
		}
	}
}
// 【界面】：保姆时段设置
void nurse_time_settings(void){
	uint8_t u8_temp;
	u8_temp = unlock_time_fun();
	if (u8_temp == USER_CONFIRM){
		Disp_sentence(25,2,"设置成功",1);
		if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
		delay_ms(1000);
	}
}
// 【界面】：数据统计及记录
void data_note(void){
	uint8_t key,pos=0;

	unlock_notes_disp(0);

	while (1){
		key = Wait_Key();

		// 如果按下4，就尝试查看上4条历史记录，看到头了就不响应
		if (key == 0x04){
			// 刷新显示区间
			if (pos>0){
				pos--;
				// 开始显示
				unlock_notes_disp(pos);
			}
		}
		// 如果按下6，就尝试查看下4条历史记录，看到头了就不响应
		else if (key == 0x06){
			// 刷新显示区间
			if (pos < ((UNLOCK_NOTES_SIZE/4)+((UNLOCK_NOTES_SIZE%4)>0))-1 ){
				pos++;
				// 开始显示
				unlock_notes_disp(pos);
			}
		}
		// 如果按下*，return
		else if (key == 0x0a){
			return;
		}
		// 如果超时，return
		else if (key == NO_KEY){
			Disp_sentence(48,2,"超时",1);
			return;
		}
	}
}



/******************************************* 底层函数 *******************************************/
// func:	向 flash 指定位置写入 ID 以及 时间数据
// addr:	历史记录的首地址，就是那个固定的全局变量 UNLOCK_NOTES_ADDR
// user_id:	写入的用户编号
void unlock_notes_write(u32 addr, uint16_t user_id){
	UNLOCK_NOTES notes;
	UNLOCK_NOTES_AREA notes_area;

	// 获取最新的历史记录信息
	notes.flag = UNLOCK_NOTES_USED_FLAG;
	notes.year = 	calendar.w_year;
	notes.month_date = (calendar.w_date<<8) | calendar.w_month;
	notes.hour_minute = (calendar.min<<8) | calendar.hour;
	notes.user_number = user_id;

	// 向下翻滚数据
	STMFLASH_Read(UNLOCK_NOTES_ADDR, (u16*)&notes_area, UNLOCK_NOTES_MSG_LENGTH*(UNLOCK_NOTES_SIZE-1));
	STMFLASH_Write(UNLOCK_NOTES_ADDR+2*UNLOCK_NOTES_MSG_LENGTH, (u16*)&notes_area, UNLOCK_NOTES_MSG_LENGTH*(UNLOCK_NOTES_SIZE-1));

	// 在开头加入最新的历史数据
	STMFLASH_Write(UNLOCK_NOTES_ADDR, (u16*)&notes, UNLOCK_NOTES_MSG_LENGTH);
}

// func:	显示4行历史记录信息
// pos:		每四条历史记录为一个pos，第【1234】条历史记录对应的pos=0
void unlock_notes_disp(u8 pos){
	char disp1[16];

	u32 addr;
	u16 flag,id;
	u8 month,date,hour,min;

	for (u8 i=0; i<4; i++){
		addr	= UNLOCK_NOTES_ADDR + pos*40 + i*10;
		flag 	= *(u16*)(addr);
		// year 	= *(u16*)(addr+2);
		month 	= *(u8*)(addr+4);
		date 	= *(u8*)(addr+5);
		hour 	= *(u8*)(addr+6);
		min 	= *(u8*)(addr+7);
		id 		= *(u16*)(addr+8);

		// 如果这条信息是真实的【历史记录】，那么就开始打印
		if (flag == 0x6666){
			// 格式如：02-28
			disp1[0] = 48 + (month)/10;
			disp1[1] = 48 + (month)%10;
			disp1[2] = '-';
			disp1[3] = 48 + (date)/10;
			disp1[4] = 48 + (date)%10;
			disp1[5] = ' ';
			// 18:31
			disp1[6] = 48 + (hour)/10;
			disp1[7] = 48 + (hour)%10;
			disp1[8] = ':';
			disp1[9] = 48 + (min)/10;
			disp1[10] = 48 + (min)%10;
			disp1[11] = ' ';
			// User Number
			disp1[12] = 48 + (id)/100;
			disp1[13] = 48 + (id/10)%10;
			disp1[14] = 48 + (id)%10;
			// 插入结束符
			disp1[15] = 0;

			// 打印在 OLED 上
			Disp_sentence_singleline(0,i*2,disp1,1);
		}
		else {
			Disp_sentence_singleline(0,i*2,"空",1);
		}
	}
}

// func:	清除所有的历史记录，将所有历史数据均置为 FF
void unlock_notes_set_all_ff(void){
	for (u16 i=0; i<UNLOCK_NOTES_SIZE; i++){
		for (u8 j=0; j<UNLOCK_NOTES_MSG_LENGTH; j++){
			Test_Write(UNLOCK_NOTES_ADDR+i*UNLOCK_NOTES_MSG_LENGTH*2+j*2, 0xFFFF);
		}
	}
}

// func:	隐藏所有的历史记录，仅将所有历史数据的标志位均置为 0xFF
void unlock_notes_set_all_flag_ff(void){
	for (u16 i=0; i<UNLOCK_NOTES_SIZE; i++){
		Test_Write(UNLOCK_NOTES_ADDR+i*UNLOCK_NOTES_MSG_LENGTH*2, 0xFFFF);
	}
}

// 参数：
// my_user_type:用户类型
// user_num：用户编号
// 返回值：
// USER_LOGIN_TIMEOUT 超时
// USER_LOGIN_SUCCESS 录入成功
uint8_t user_login(user_type my_user_type,uint16_t user_num){
	uint8_t temp_u8 = 0x00;
	uint8_t key;
	uint8_t ID;
	uint16_t user_amount;
	uint8_t key_length;
	int i;
	int n=0;
	MY_USER user_temp;
	uint8_t rfcard;
	uint32_t RFCARD_ID;

	// 初始化 user_temp
	user_temp.password_length=0;//这两行初始化很重要，之后的程序，可以通过指纹id和密码长度判断这个用户是否具有指纹或者密码。其它的在录入成功时再填入数据
	user_temp.finger_number=0;
	user_temp.rfcard_id = 0x00000000;
	QS808_Rec_Buf_refresh();
	while(1)
	{
		n++;
		if(n>WAIT_TIME_MS/90)
				return USER_LOGIN_TIMEOUT;
		if (QS808_Rec_Buf.Trans_state == reset)
		{
			QS808_Detect_Finger();
		}
		if(QS808_Rec_Buf.Rec_state == busy)//这说明收完或者正在收一帧
		{
			uint8_t temp = QS808_Detect_Finger_Unpack();//解析这一帧
			if(temp == ERR_FINGER_DETECT)
			{
				temp_u8 = QS808_Login(&ID);		//获取指纹的id，并且返回
				switch(temp_u8)
				{
					case QS808_WAIT_TIMEOUT:	//等待超时，返回主菜单
					{
							return USER_LOGIN_TIMEOUT;
					}
					case ERR_BAD_QUALITY:		//指纹质量差，重新录入
					{
							Disp_sentence(20,2,"指纹质量差",1);
							Disp_sentence(20,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							 OLED_CLS();
							return LOGIN_ERROR;
					}
					case ERR_MERGE_FAIL:		//指纹合成失败，重新录入
					{
							Disp_sentence(20,2,"录入失败",1);
							Disp_sentence(20,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							 OLED_CLS();
							return LOGIN_ERROR;
					}
					case ERR_DUPLICATION_ID:	//已存在此手指，重新录入
					{
							Disp_sentence(20,2,"指纹已存在",1);
							Disp_sentence(20,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return LOGIN_ERROR;
					}
					case QS808_DATA_ERR:
					{
							Disp_sentence(0,2,"录入失败",1);
							Disp_sentence(0,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//清屏
							return LOGIN_ERROR;
					}
					case ERR_SUCCESS:			//成功，并且存储这个用户
					{
						user_temp.flag = user_flag_value;
						user_temp.number = user_num;
						user_temp.password_length = 0;
						user_temp.finger_number = ID;
						user_temp.my_user_type = my_user_type;
						user_struct_store(user_num,(uint16_t *) &user_temp);

						//更新用户个数
						if(my_user_type == admin)//管理员
						{
							user_amount = *(vu16*)admin_amount_addr;
							user_amount++;
							STMFLASH_Write(admin_amount_addr,&user_amount,1);
						}
						else if(my_user_type == family)//家人
						{
								user_amount = *(vu16*)family_amount_addr;
								user_amount++;
								STMFLASH_Write(family_amount_addr,&user_amount,1);
						}
						else if (my_user_type == babysitter)//保姆
						{
								user_amount = *(vu16*)babysitter_amount_addr;
								user_amount++;
								STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
						}
						user_amount = *(vu16*)alluser_amount_addr;//用户总数加1
						user_amount++;
						STMFLASH_Write(alluser_amount_addr,&user_amount,1);
						return USER_LOGIN_SUCCESS;
					}
					default:
					{
							Disp_sentence(0,2,"录入失败",1);
							Disp_sentence(0,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//清屏
							return LOGIN_ERROR;
					}
				}
			}
		}


		//扫描射频卡
		////////////////////////////////////////////////////////////////////射频卡重复性检测没有
		rfcard = IC_test (&RFCARD_ID);
		if (rfcard == RFCARD_DETECED)
		{
			user_temp.flag = user_flag_value;
			user_temp.my_user_type = my_user_type;
			user_temp.number = user_num;
			user_temp.rfcard_id = RFCARD_ID;
			user_struct_store(user_num,(uint16_t *) &user_temp);
			//更新用户个数
			if(my_user_type == admin)//管理员
			{
				user_amount = *(vu16*)admin_amount_addr;
				user_amount++;
				STMFLASH_Write(admin_amount_addr,&user_amount,1);
			}
			else if(my_user_type == family)//家人
			{
				user_amount = *(vu16*)family_amount_addr;
				user_amount++;
				STMFLASH_Write(family_amount_addr,&user_amount,1);
			}
			else if (my_user_type == babysitter)//保姆
			{
				user_amount = *(vu16*)babysitter_amount_addr;
				user_amount++;
				STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
			}
			user_amount = *(vu16*)alluser_amount_addr;//用户总数加1
			user_amount++;
			STMFLASH_Write(alluser_amount_addr,&user_amount,1);
	//				Disp_sentence(0,2,"录入成功",1);
			return USER_LOGIN_SUCCESS;
		}
		//扫描按键
		key = IsKey();//时间不明
		if (key == NO_KEY)
		{
			continue;
		}
		else if (key == 0x0a)//星号,返回
		{
	//		printf("返回\n");
			return USER_BACK;
		}
		else if (key == 0x0b)//井号，不反应
		{
			continue;
		}
		else
		{
			temp_u8 = Key_Cap_6bits(key,&key_length,0);//进入按键捕捉，记录密码														/////////////////////////
			if(temp_u8 == KEY_CANCEL)
			{
	//			printf("返回\n");
				return USER_BACK;
			}
			else if (temp_u8 == KEY_TIMEOUT)
			{
	//					Disp_sentence(0,2,"超时",1);
				return USER_LOGIN_TIMEOUT;
			}
			else if (temp_u8 == KEY_TOO_LONG)//密码过长
			{
				Disp_sentence(25,2,"密码太长",1);
				Disp_sentence(18,4,"请重新录入",0);
				delay_ms(1000);
				return LOGIN_ERROR;
			}
			else if (temp_u8 == KEY_CONFIRM)//密码输入完成
			{
				//检查是否已经有此密码，两个用户不能用相同的密码，因为当保姆和家人密码相同时，导致保姆的时限失效
				user_type user_type1;
				temp_u8 = password_verify_2unlock(key_length,&user_type1);
				if (temp_u8 == VERIFF_SUCCESS)//说明密码已经存在
				{
					Disp_sentence(25,2,"密码已存在",1);
					Disp_sentence(25,4,"请重新录入",0);
					delay_ms(1000);
					return LOGIN_ERROR;
				}

				user_temp.flag = user_flag_value;
				user_temp.my_user_type = my_user_type;
				user_temp.number = user_num;
				user_temp.password_length = key_length;
				for (i=0;i<key_length;i++)
					user_temp.password[i] = Key_Buffer[i];
				user_temp.finger_number = 0;
				user_struct_store(user_num,(uint16_t *) &user_temp);
				//更新用户个数
				if(my_user_type == admin)//管理员
				{
					user_amount = *(vu16*)admin_amount_addr;
					user_amount++;
					STMFLASH_Write(admin_amount_addr,&user_amount,1);
				}
				else if(my_user_type == family)//家人
				{
					user_amount = *(vu16*)family_amount_addr;
					user_amount++;
					STMFLASH_Write(family_amount_addr,&user_amount,1);
				}
				else if (my_user_type == babysitter)//保姆
				{
					user_amount = *(vu16*)babysitter_amount_addr;
					user_amount++;
					STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
				}
				user_amount = *(vu16*)alluser_amount_addr;//用户总数加1
				user_amount++;
				STMFLASH_Write(alluser_amount_addr,&user_amount,1);
	//				Disp_sentence(0,2,"录入成功",1);
				return USER_LOGIN_SUCCESS;

			}
		}

	}

}

//my_user_type:用户类型
//user_num：用户编号
//返回值：
//USER_LOGIN_TIMEOUT 超时
//USER_LOGIN_SUCCESS 修改成功
uint8_t user_modify(uint16_t user_num){
	uint8_t temp_u8 = 0x00;
	uint8_t key;
	uint8_t ID;
	uint8_t ID2;//缓存修改之前的指纹ID
	uint8_t key_length;
	int i;
	int n=0;
	MY_USER user_temp;
	uint32_t addr;
	uint8_t rfcard;
	uint32_t RFCARD_ID;
	// printf("按手指或输入密码\n");
	// 先把用户结构体缓存出来
	for (i=0;i<MY_USER_MAX_NUM;i++)
	{
		addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
		STMFLASH_Read(addr,(uint16_t*)&user_temp,MY_USER_length/2); //注意第三个参数是半字数，这很关键
		if (user_temp.flag == 0xAA)//是用户
			if(user_temp.number == user_num)//找到了当前编号
				break;//跳出，后面在user_temp上进行修改，然后保存即可
			//这里不考虑编号不存在的问题，这个风险在上一级处理过了
	}
	ID2 = user_temp.finger_number;
	QS808_Rec_Buf_refresh();
	while(1)
	{
		n++;
		if(n>WAIT_TIME_MS/90)
				return USER_LOGIN_TIMEOUT;
		if (QS808_Rec_Buf.Trans_state == reset)
		{
			QS808_Rec_Buf.Trans_state = set;
			QS808_Detect_Finger();
		}
		if(QS808_Rec_Buf.Rec_state == busy)//这说明收完或者正在收一帧
		{
				uint8_t temp = QS808_Detect_Finger_Unpack();//解析这一帧
				if(temp == ERR_FINGER_DETECT)
				{
					temp_u8 = QS808_Login(&ID);//返回指纹录入时的相关信息，如：指纹质量不佳，等待超时等等
					switch(temp_u8)
					{
						case QS808_WAIT_TIMEOUT://等待超时，返回主菜单，return USER_LOGIN_TIMEOUT;
						{
							return USER_LOGIN_TIMEOUT;
						}
						case ERR_BAD_QUALITY:	//指纹质量差，重新录入，return LOGIN_ERROR;
						{
							Disp_sentence(0,2,"指纹质量差",1);
							Disp_sentence(0,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return LOGIN_ERROR;
						}
						case ERR_MERGE_FAIL:	//指纹合成失败，重新录入，return LOGIN_ERROR;
						{
							Disp_sentence(0,2,"录入失败",1);
							Disp_sentence(0,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//清屏
							return LOGIN_ERROR;
						}
						case ERR_DUPLICATION_ID://已存在此手指，重新录，return LOGIN_ERROR;
						{
							Disp_sentence(0,2,"指纹已存在",1);
							Disp_sentence(0,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();
							return LOGIN_ERROR;
						}
						case QS808_DATA_ERR:
						{
							Disp_sentence(0,2,"录入失败",1);
							Disp_sentence(0,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//清屏
							return LOGIN_ERROR;
						}
						case ERR_SUCCESS://成功，存储用户，return USER_LOGIN_SUCCESS;
						{
							user_temp.finger_number = ID;//只修改指纹ID 其它不变
							STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//结构体保存

							//不用更新用户个数
							//在QS808中删除原有指纹
							if (ID2!=0)//这说明这个用户之前有指纹，需要再qs808中删除这个用户的指纹
								QS808_CMD_DEL_NUM2(ID2);
							return USER_LOGIN_SUCCESS;
						}
						default:
						{
							Disp_sentence(0,2,"录入失败",1);
							Disp_sentence(0,4,"请重新录入",0);
							if(!SPEAK_flag) SPEAK_OPT_FAIL();
							delay_ms(1000);
							OLED_CLS();//清屏
							return LOGIN_ERROR;
						}
					}
				}
		}
		//扫描射频卡
		uint8_t rfcard_repeat_flag=0;
		rfcard = IC_test (&RFCARD_ID);
		if (rfcard == RFCARD_DETECED)
		{
			//检查是否已经有射频卡
			MY_USER my_user1;
			for (i=0;i<MY_USER_MAX_NUM;i++)
			{
				addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
				STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
				if (my_user1.flag == 0xAA)//判断是否有有效数据
				{
					if (my_user1.rfcard_id == RFCARD_ID)
						rfcard_repeat_flag = 1;
				}
			}
			if (rfcard_repeat_flag)//重复
			{
				Disp_sentence(0,2,"卡已存在",1);
				Disp_sentence(0,4,"请重新录入",0);
				if(!SPEAK_flag) SPEAK_OPT_FAIL();
				delay_ms(1000);
				return LOGIN_ERROR;
			}
			user_temp.rfcard_id = RFCARD_ID;//修改密码长度，与密码内容
			STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//结构体保存
			return USER_LOGIN_SUCCESS;
		}
		//按键部分
		key = IsKey();//时间不明
		if (key == NO_KEY)
		{
			continue;
		}
		else if (key == 0x0a)//星号,返回
		{
	//		printf("返回\n");
			return USER_BACK;
		}
		else if (key == 0x0b)//井号，不反应
		{
			continue;
		}
		else
		{
			temp_u8 = Key_Cap_6bits(key,&key_length,0);//进入按键捕捉，记录密码
			if(temp_u8 == KEY_CANCEL)
			{
	//			printf("返回\n");
				return USER_BACK;
			}
			else if (temp_u8 == KEY_TIMEOUT)
			{
	//					Disp_sentence(0,2,"超时",1);
				return USER_LOGIN_TIMEOUT;
			}
			else if (temp_u8 == KEY_TOO_LONG)//密码过长
			{
				Disp_sentence(0,2,"密码太长",1);
				Disp_sentence(0,4,"请重新录入",0);
				if(!SPEAK_flag) SPEAK_OPT_FAIL();
				delay_ms(1000);
				return LOGIN_ERROR;
			}
			else if (temp_u8 == KEY_CONFIRM)//密码输入完成
			{
				//检查是否已经有此密码，两个用户不能用相同的密码，因为当保姆和家人密码相同时，导致保姆的时限失效
				user_type user_type1;
				temp_u8 = password_verify_2unlock(key_length,&user_type1);
				if (temp_u8 == VERIFF_SUCCESS)//说明密码已经存在
				{
					Disp_sentence(0,2,"密码已存在",1);
					Disp_sentence(0,4,"请重新录入",0);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					delay_ms(1000);
					return LOGIN_ERROR;
				}
				user_temp.password_length = key_length;//修改密码长度，与密码内容
				for (i=0;i<key_length;i++)
					user_temp.password[i] = Key_Buffer[i];
				STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//结构体保存
				return USER_LOGIN_SUCCESS;
			}
		}

	}
}

// 【功能】删除用户
void user_delete(user_type my_user_type,uint16_t user_num){
	//当user_num为0xFFFF时，删除全部 my_user_type 类型用户 否则删除编号为user_num的用户
	MY_USER user_temp;
	uint32_t addr;
	uint16_t user_amount,user_amount2;//用户数量
	int i;
	if (user_num == 0xffff)//全部删除
	{
		for (i=0;i<MY_USER_MAX_NUM;i++)
		{
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr,(uint16_t*)&user_temp,MY_USER_length/2); //注意第三个参数是半字数，这很关键
			if (user_temp.flag == 0xAA)
			{
				if (user_temp.my_user_type == my_user_type)
				{
					if(user_temp.finger_number != 0x00)//说明用户有指纹，要在qs808中删除
					{
						QS808_CMD_DEL_NUM2(user_temp.finger_number);
					}
					user_temp.flag = 0x00;//其实只需要flag清掉就行了，其它是以防万一
					user_temp.number = 0;
					user_temp.finger_number = 0;
					user_temp.password_length = 0;
					user_temp.rfcard_id = 0x00000000;
					STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//结构体保存

					//用户计数修改
					if (my_user_type == admin)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//总数
						user_amount2 =  *(uint16_t*)admin_amount_addr;//管理员数
						user_amount = user_amount - user_amount2;//新总数
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//保存总数
						user_amount = 0;//新管理员数
						STMFLASH_Write((uint32_t)admin_amount_addr,(uint16_t*)&user_amount,1);
					}
					else if (my_user_type == family)//其它类型 待编辑
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//总数
						user_amount2 =  *(uint16_t*)family_amount_addr;
						user_amount = user_amount - user_amount2;//新总数
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//保存总数
						user_amount = 0;
						STMFLASH_Write((uint32_t)family_amount_addr,(uint16_t*)&user_amount,1);
					}
					else if (my_user_type == babysitter)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//总数
						user_amount2 =  *(uint16_t*)babysitter_amount_addr;
						user_amount = user_amount - user_amount2;//新总数
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//保存总数
						user_amount = 0;
						STMFLASH_Write((uint32_t)babysitter_amount_addr,(uint16_t*)&user_amount,1);
					}

				}
			}
		}

	}
	else
	{
		for (i=0;i<MY_USER_MAX_NUM;i++)
		{
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr,(uint16_t*)&user_temp,MY_USER_length/2); //注意第三个参数是半字数，这很关键
			if (user_temp.flag == 0xAA)
			{
				if (user_temp.number == user_num)
				{
					if(user_temp.finger_number != 0x00)//说明用户有指纹，要在qs808中删除
					{
						QS808_CMD_DEL_NUM2(user_temp.finger_number);
					}
					user_temp.flag = 0x00;//其实只需要flag清掉就行了，其它两个是以防万一
					user_temp.number = 0;
					user_temp.finger_number = 0;
					user_temp.password_length = 0;
					user_temp.rfcard_id = 0x00000000;
					STMFLASH_Write((uint32_t)addr,(uint16_t*)&user_temp,MY_USER_length/2);//结构体保存
					//计数-1
					if (my_user_type == admin)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//总数
						user_amount2 =  *(uint16_t*)admin_amount_addr;//管理员数
						user_amount--;
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//保存总数
						user_amount2--;//新管理员数
						STMFLASH_Write((uint32_t)admin_amount_addr,(uint16_t*)&user_amount2,1);
					}
					else if (my_user_type == family)//其它类型 待编辑
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//总数
						user_amount2 =  *(uint16_t*)family_amount_addr;//
						user_amount--;
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//保存总数
						user_amount2--;//
						STMFLASH_Write((uint32_t)family_amount_addr,(uint16_t*)&user_amount2,1);
					}
					else if (my_user_type == babysitter)
					{
						user_amount = *(uint16_t*)alluser_amount_addr;//总数
						user_amount2 =  *(uint16_t*)babysitter_amount_addr;//
						user_amount--;
						STMFLASH_Write((uint32_t)alluser_amount_addr,(uint16_t*)&user_amount,1);//保存总数
						user_amount2--;
						STMFLASH_Write((uint32_t)babysitter_amount_addr,(uint16_t*)&user_amount2,1);
					}

				}
			}
		}
	}


}

void user_struct_store(uint16_t user_num,uint16_t * user_struct){
	//存储方式采用连续存储，从用户结构体存储地址第一个开始搜索，找到可以存的位置，直接存储
	//这里不考虑flash存储不下的问题，在外层的函数解决过了
	int i;
	MY_USER my_user1;
	uint32_t addr;//flash地址
	for (i=0;i<MY_USER_MAX_NUM;i++)
	{
		addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
		STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2); //注意第三个参数是半字数，这很关键，这里写错，导致my_user1越界
		if (my_user1.flag != 0xAA)//这个位置没有存储用户，可以存在这里
			break;
	}
	STMFLASH_Write(addr,user_struct,MY_USER_length/2);
}

//初次使用初始化，包括：
//1.检测是否是初次使用（INIT_WORD）
//2.初始化用户数量user_amount_addr（user_amount_addr）
int first_time_init(void){
	UNLOCK_TIME unlocktime;
	uint16_t init_word0;
	uint16_t user_amount = 0;//初始化为0个用户
	uint16_t initword1 = INIT_WORD;
	init_word0 = *(uint16_t*)INIT_ADDR;
	if (init_word0 == INIT_WORD)//不是第一次使用，读取音频设置，然后return 0
	{
		STMFLASH_Read(speak_flag_addr,&SPEAK_flag,1);
		return 0;
	}
	//第一次使用
	SPEAK_flag=0;
	STMFLASH_Write(INIT_ADDR,&initword1,1);	//第一次使用，写入关键字
	STMFLASH_Write(admin_amount_addr,&user_amount,1);	//第一次使用，将用户数量初始化为0
	STMFLASH_Write(family_amount_addr,&user_amount,1);	//第一次使用，将用户数量初始化为0
	STMFLASH_Write(babysitter_amount_addr,&user_amount,1);
	STMFLASH_Write(alluser_amount_addr,&user_amount,1);
	STMFLASH_Write(speak_flag_addr,&SPEAK_flag,1);//初始化为打开语音


	unlocktime.hour = 0;
	unlocktime.hour2 = 23;
	unlocktime.minute = 0;
	unlocktime.minute2 = 59;

	STMFLASH_Write(UNLOCK_TIME_addr,(uint16_t*)&unlocktime,UNLOCK_TIME_length/2);

	uint32_t temp;
	temp = sizeof(MY_USER);//计算一下结构体长度 不是128的话 回去改结构体
	if (temp != MY_USER_length)//用户结构体长度不对
	{
		Disp_sentence(0,0,"error!",1);
		while(1);
	}

	return 1;

}

// 判断当前时间是否属于可开锁时段，用于保姆用户
uint8_t time_verify(void){
	//返回值 0 不能开门 1 可以开门
	UNLOCK_TIME unlocktime;
	u8 hour_now = calendar.hour;
	u8 min_now = calendar.min;

	STMFLASH_Read(UNLOCK_TIME_addr,(uint16_t*)&unlocktime,UNLOCK_TIME_length/2);
	uint32_t time1 = unlocktime.hour*60+unlocktime.minute;//把时间换算到分钟，这样好判断
	uint32_t time2 = unlocktime.hour2*60+unlocktime.minute2;
	uint32_t time_now = hour_now*60+min_now;

	// 正常情况，预设时间下限大于上限
	if (time2 >= time1){
		if ((time_now>=time1) && (time_now<=time2))
			return 1;
		else
			return 0;
	}
	// 预设时间下限小于上限，说明允许时间跨过了0点
	else {
		if (((time_now>=time1)&&(time_now<=1439))||(time_now<=time2))//1439=23*60+59
			return 1;
		else
			return 0;
	}
}

// 【开门时段设置】界面
uint8_t unlock_time_fun(void){
	uint8_t key;
	u8 hour1,min1;
	u8 hour2,min2;
	int i=0;
	int j=0;
	UNLOCK_TIME unlocktime;
	Disp_sentence(0,0,"请输入开门时间",1);
	while(1)
	{
		i=0;
		j=0;

		uint8_t arr[8]={0,0,0,0,2,3,5,9};//时间
		uint8_t site[8] = {0,8,24,32,48,56,72,80};//显示坐标
		hour1 = 10*arr[0]+arr[1];
		min1 = 10*arr[2]+arr[3];
		hour2 = 10*arr[4]+arr[5];
		min2 = 10*arr[6]+arr[7];
		//初始化显示
		for (j=0;j<8;j++)
		{
			if (j==0)
				OLED_Show_inverse(site[j],2,arr[j]+48);
			else
				OLED_Show(site[j],2,arr[j]+48,0,1);
		}
		Disp_sentence(16,2,":",0);
		Disp_sentence(40,2,"~",0);
		Disp_sentence(64,2,":",0);
		while(1)
		{
			key = Wait_Key();
			if (key == NO_KEY)
				return USER_LOGIN_TIMEOUT;
			else if (key==0x0a)//星号
			{
				if(i==0)
					return USER_BACK;
				else
					i--;
			}
			else if (key==0x0b)//确认
			{
				//检查合法性
				if ((hour1>23)||(hour2>23)||(min1>59)||(min1>59))
				{
					Disp_sentence(25,2,"设置有误",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					delay_ms(1000);
					OLED_CLS();
					break;
				}
				//设置
				unlocktime.hour = hour1;
				unlocktime.hour2 = hour2;
				unlocktime.minute = min1;
				unlocktime.minute2 = min2;
				STMFLASH_Write(UNLOCK_TIME_addr,(uint16_t*)&unlocktime,UNLOCK_TIME_length/2);//存储
				return USER_CONFIRM;
			}
			else //用户输入数字，更新
			{
				arr[i] = key;
				if(i<7)//光标不在最右边，光标右移
					i++;
				hour1 = 10*arr[0]+arr[1];
				min1 = 10*arr[2]+arr[3];
				hour2 = 10*arr[4]+arr[5];
				min2 = 10*arr[6]+arr[7];
			}
			//重新绘制界面
			for (j=0;j<8;j++)
			{
				if (j==i)//反色显示
					OLED_Show_inverse(site[j],2,arr[j]+48);
				else //正色显示
					OLED_Show(site[j],2,arr[j]+48,0,1);
			}
		}
	}

}

// 【系统时间设置】界面
uint8_t time_settings(void){
	int j=0;
	int i=0;
	uint8_t key;
	uint8_t leap_year;

	while(1){
		uint8_t arr[12]={2,0,1,5,0,1,0,1,0,0,0,0};	//时间
		uint8_t sitex[12] = {0,8,16,24,48,56,80,88,0,8,32,40};	//显示数字的横坐标矩阵
		uint8_t sitey[12] = {2,2,2,2,2,2,2,2,4,4,4,4};	//显示数字的纵坐标矩阵
		u8 mon_table[12]={31,29,31,30,31,30,31,31,30,31,30,31};	//月份表
		u16 year;
		u8 smon,sday,hour,min;
		year = arr[0]*1000+ arr[1]*100+ arr[2]*10+ arr[3];	//年
		smon = arr[4]*10+ arr[5];
		sday = arr[6]*10+ arr[7];
		hour = arr[8]*10+ arr[9];
		min = arr[10]*10+ arr[11];
		// RTC_Set(2015,1,1,0,0,0);  //设置时间

		// 初始化显示
		OLED_CLS();
		for (j=0;j<12;j++){
			// 反色显示
			if (j==0)
				OLED_Show_inverse(sitex[j],sitey[j],arr[j]+48);
			// 正色显示
			else
				OLED_Show(sitex[j],sitey[j],arr[j]+48,0,1);
		}
		Disp_sentence(32,2,"年",0);
		Disp_sentence(64,2,"月",0);
		Disp_sentence(96,2,"日",0);
		Disp_sentence(16,4,"时",0);
		Disp_sentence(48,4,"分",0);

		// 时间设置
		i=0;
		while(1){
			key = Wait_Key();

			// 没有按下按键
			if (key == NO_KEY){
				return USER_LOGIN_TIMEOUT;
			}

			// *，撤回
			else if (key==0x0a){
				if(i==0)
					return USER_BACK;
				else
					i--;
			}

			// #，确认
			else if (key==0x0b){
				// 检查合法性
				if ((smon>12)||(hour>23)||(min>59))//月 时 分的不合法情况
				{
					Disp_sentence(0,0,"设置有误",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					// flag = 1;
					delay_ms(1000);
					break;
				}
				if (sday>mon_table[smon-1])//日 不合法
				{
					Disp_sentence(0,0,"设置有误",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					// flag = 1;
					delay_ms(1000);
					break;
				}
				if((!leap_year)&&(smon==2)&&(sday>28))//平年2月不合法
				{
					Disp_sentence(0,0,"设置有误",1);
					if(!SPEAK_flag) SPEAK_OPT_FAIL();
					// flag = 1;
					delay_ms(1000);
					break;
				}
				RTC_Set(year,smon,sday,hour,min,0);
				return USER_CONFIRM;
			}

			// 用户输入数字，更新arr
			else {
				arr[i] = key;
				if(i<11)//光标不在最右边，光标右移
					i++;
				year = arr[0]*1000+ arr[1]*100+ arr[2]*10+ arr[3];//年
				smon = arr[4]*10+ arr[5];
				sday = arr[6]*10+ arr[7];
				hour = arr[8]*10+ arr[9];
				min = arr[10]*10+ arr[11];
				leap_year = Is_Leap_Year(year);
			}

			// 重新绘制界面
			for (j=0;j<12;j++){
				if (j==i)//反色显示
					OLED_Show_inverse(sitex[j],sitey[j],arr[j]+48);
				else //正色显示
					OLED_Show(sitex[j],sitey[j],arr[j]+48,0,1);
			}
		}

	}
}

// 【新版】验证管理员函数
uint8_t admin_verify(void){
	uint8_t temp_u8;
	uint8_t key_length;
	uint8_t finger_num;
	uint8_t key;
	uint32_t RFCARD_ID;
	uint32_t while_cnt=0;
	uint32_t addr;
	int i;
	MY_USER my_user1;
	QS808_Rec_Buf_refresh();
	while(1){
		while_cnt++;
		// 如果超时，就 return USER_LOGIN_TIMEOUT
		if (while_cnt == WAIT_TIME_MS/100){
			//这个while（1）每圈耗时100ms
			return USER_LOGIN_TIMEOUT;
		}
		// 如果还没有向指纹头发送过数据，就发送寻找手指指令
		if (QS808_Rec_Buf.Trans_state == reset){
			QS808_Rec_Buf.Trans_state = set;
			// 发送寻找手指指令，只发送不接收
			QS808_Detect_Finger();
		}

		// 扫描按键
		key = IsKey();
		// 如果没有按键按下，就开始检查指纹或者射频卡
		if (key == NO_KEY){
			// 如果busy，这说明指纹头收完或者正在收一帧
			if (QS808_Rec_Buf.Rec_state == busy){
				// 解析这一帧
				uint8_t temp = QS808_Detect_Finger_Unpack();
				// 如果指纹获取成功，就进行指纹查询
				if (temp == ERR_FINGER_DETECT){
					// 进行指纹查询
					temp_u8 = QS808_SEARCH(&finger_num);
					// 如果指纹查询超时，就播报，清屏，return
					if (temp_u8 == QS808_WAIT_TIMEOUT){
						Disp_sentence(48,4,"超时",1);
						delay_ms(1000);
						OLED_CLS();
						return USER_LOGIN_TIMEOUT;
					}
					// 如果指纹查询成功
					else if (temp_u8 == ERR_SUCCESS){
						// 验证指纹是否属于管理员
						for (i=0; i<MY_USER_MAX_NUM; i++){
							addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
							STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
							// 如果这是已录入的用户
							if (my_user1.flag == 0xAA){
								// 如果验证到
								if (my_user1.finger_number ==  finger_num){
									// 如果这个人是管理员，播报，清屏，return VERIFF_SUCCESS
									if (my_user1.my_user_type == admin){
										Disp_sentence(32,2,"验证成功",1);
										if(!SPEAK_flag) SPEAK_OPT_SUCCESS();
										delay_ms(1000);
										OLED_CLS();
										return VERIFF_SUCCESS;
									}
									// 如果这个人不是管理员，播报，清屏，return VERIFF_FAIL
									else{
										Disp_sentence(32,2,"验证失败",1);
										Disp_sentence(24,4,"请重新验证",0);
										if(!SPEAK_flag) SPEAK_OPT_FAIL();
										delay_ms(1000);
										OLED_CLS();
										return VERIFF_FAIL;
									}
								}
							}
						}
					}
					// 不论是指纹库为空，指纹不存在，指纹质量不好都归为验证失败，不然编的累死了
					else{
						Disp_sentence(32,2,"验证失败",1);
						Disp_sentence(24,4,"请重新验证",0);
						delay_ms(1000);
						OLED_CLS();
						return VERIFF_FAIL;;
					}
				}
			}

			// 检测射频卡
			temp_u8 = IC_test (&RFCARD_ID);
			// 检测到射频卡
			if (temp_u8 == RFCARD_DETECED)
			{
					for (i=0;i<MY_USER_MAX_NUM;i++)
					{
						addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
						STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
						if (my_user1.flag == 0xAA)
						{
							if (my_user1.rfcard_id ==  RFCARD_ID)//验证到了射频卡
							{
								if (my_user1.my_user_type == admin)//管理员才能解锁设置模式
								{
									Disp_sentence(32,2,"验证成功",1);
									delay_ms(1000);
									return VERIFF_SUCCESS;
								}
								else //非管理员
								{
										Disp_sentence(32,2,"验证失败",1);
										Disp_sentence(24,4,"请重新验证",0);
										delay_ms(1000);
										OLED_CLS();
										return VERIFF_FAIL;;
								}

							}
						}
					}
					//运行到这里 说明此卡没有录入过 或者不是管理员
					Disp_sentence(32,2,"验证失败",1);
					Disp_sentence(24,4,"请重新验证",0);
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
		}
		// 如果按下 * ，就 return USER_BACK
		else if (key == 0x0a){
			return USER_BACK;
		}
		// 如果按下 # ，什么都不做
		else if (key == 0x0b){
		}
		// 如果按下数字键，就开始进行密码验证
		else {
			// 进入按键捕捉，记录密码，
			// temp_u8 = Key_Cap(key,&key_length,1);
			temp_u8 = admin_verify3_key_cap(key,&key_length);
			// 如果按下 * ，就 return USER_BACK
			if(temp_u8 == KEY_CANCEL){
				return USER_BACK;
			}
			// 如果超时，就播报，清屏，return USER_LOGIN_TIMEOUT
			else if (temp_u8 == KEY_TIMEOUT){
				Disp_sentence(48,2,"超时",1);
				delay_ms(1000);
				OLED_CLS();
				return USER_LOGIN_TIMEOUT;
			}
			// 如果密码过长，就播报，清屏，continue
			else if (temp_u8 == KEY_TOO_LONG){
				Disp_sentence(32,2,"密码太长",1);
				Disp_sentence(24,4,"请重新录入",0);
				delay_ms(1000);
				OLED_CLS();
				continue;
			}
			// 如果密码输入完成，就开始进行密码验证
			else if (temp_u8 == KEY_CONFIRM){
				// 密码验证函数
				temp_u8 = password_verify_2admin(key_length);
				// 如果验证失败，就播报，清屏，return VERIFF_FAIL
				if (temp_u8 == VERIFF_FAIL){
					Disp_sentence(32,2,"验证失败",1);
					Disp_sentence(24,4,"请重新验证",0);
					delay_ms(1000);
					OLED_CLS();
					return VERIFF_FAIL;
				}
				// 如果验证成功，就播报，return VERIFF_SUCCESS
				else {
					Disp_sentence(32,2,"验证成功",1);
					delay_ms(1000);
					return VERIFF_SUCCESS;
				}
			}
		}
	}
}

// 检测输入的密码是否正确，【用于进入管理员模式】
// key_length	输入参数是当前密码的长度
// 支持‘虚码’功能
uint8_t password_verify_2admin(uint8_t key_length){
	char ans = VERIFF_FAIL;
	uint8_t key_temp[6]={NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY};

	// 如果输入的密码为6位，那就直接开始检测密码的正确性
	if (key_length==6){
		ans = password_verify_only6_admin(6, key_temp);
	}
	// 如果输入的密码长于6位，那就开始循环测试密码正确性
	else if (key_length>6){
		for (u8 i=0; i<=key_length-6; i++){
			// 截取6位密码给 key_temp
			for (u8 j=0; j<6; j++){
				key_temp[j] = Key_Buffer[j+i];
			}
			// 对 key_temp 进行密码正确性验证
			ans = password_verify_only6_admin(6, key_temp);
			if (VERIFF_SUCCESS == ans){
				break;
			}
		}
	}
	return ans;
}

// 检测输入的密码是否正确，【用于解锁】
// key_length	输入参数是当前密码的长度
// 支持‘虚码’功能
uint8_t password_verify_2unlock(uint8_t key_length, user_type *user_type_o){
	*user_type_o = error;
	char ans = VERIFF_FAIL;
	uint8_t key_temp[6]={NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY,NO_KEY};

	// 如果输入的密码为6位，那就直接开始检测密码的正确性
	if (key_length==6){
		ans = password_verify_only6(6, user_type_o, Key_Buffer);
	}
	// 如果输入的密码长于6位，那就开始循环测试密码正确性
	else if (key_length>6){
		for (u8 i=0; i<=key_length-6; i++){
			// 截取6位密码给 key_temp
			for (u8 j=0; j<6; j++){
				key_temp[j] = Key_Buffer[j+i];
			}
			// 对 key_temp 进行密码正确性验证
			ans = password_verify_only6(6, user_type_o, key_temp);
			if (VERIFF_SUCCESS == ans){
				break;
			}
		}
	}
	return ans;
}

// 检测输入的密码是否正确，仅仅支持6位数的认证
// 函数比较全局变量Key_Buffer与flash中存储的密码是否相同
// 输出验证结果，和用户类型
uint8_t password_verify_only6(uint8_t key_length, user_type *user_type_o, uint8_t *key_ans){
	*user_type_o = error;
	int i=0;
	int j=0;
	int flag=0;	//密码匹配标志，0表示失败 1表示成功


	// 缓存flash中读出的用户结构体
	uint32_t addr = (uint32_t)MY_USER_addr_base;
	MY_USER * my_user1;
	my_user1 = (MY_USER*)malloc(sizeof(MY_USER));

	// 如果输入的密码长度大于6位，就 return VERIFF_FAIL
	if (key_length != 6){
		return VERIFF_FAIL;
	}

	// 如果密码长度等于6位，那就开始检测密码是否正确
	else {
		for (i=0; i<MY_USER_MAX_NUM; i++){
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr, (uint16_t*)my_user1, MY_USER_length/2);
			// 判断是否有有效数据
			if (my_user1->flag != 0xAA)
				continue;
			// 开始比较
			while (j<6){
				if (my_user1->password[j] != key_ans[j]){
					flag = 0;
					break;
				}
				else {
					flag = 1;
				}
				j++;
			}
			// 密码正确
			if (flag){
				// *user_type_o = my_user1->my_user_type;
				free(my_user1);
				my_user1 = NULL;
				return VERIFF_SUCCESS;
			}
		}
		free(my_user1);
		my_user1 = NULL;
		return VERIFF_FAIL;
	}
}
// 上面函数的【只有管理员才可以通过】版本
uint8_t password_verify_only6_admin(uint8_t key_length, uint8_t *key_ans){
	int i=0;
	int j=0;
	int flag=0;	//密码匹配标志，0表示失败 1表示成功


	// 缓存flash中读出的用户结构体
	uint32_t addr = (uint32_t)MY_USER_addr_base;
	MY_USER * my_user1;
	my_user1 = (MY_USER*)malloc(sizeof(MY_USER));

	// 如果输入的密码长度大于6位，就 return VERIFF_FAIL
	if (key_length != 6){
		return VERIFF_FAIL;
	}

	// 如果密码长度等于6位，那就开始检测密码是否正确
	else {
		for (i=0; i<MY_USER_MAX_NUM; i++){
			addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
			STMFLASH_Read(addr, (uint16_t*)my_user1, MY_USER_length/2);
			// 判断是否有有效数据
			if (my_user1->flag != 0xAA)
				continue;
			if (my_user1->my_user_type != admin)//用户不是管理员（防止用户输入了普通密码而进入设置模式）
				continue;
			// 开始比较
			while (j<6){
				if (my_user1->password[j] != key_ans[j]){
					flag = 0;
					break;
				}
				else {
					flag = 1;
				}
				j++;
			}
			// 密码正确
			if (flag){
				// *user_type_o = my_user1->my_user_type;
				free(my_user1);
				my_user1 = NULL;
				return VERIFF_SUCCESS;
			}
		}
		free(my_user1);
		my_user1 = NULL;
		return VERIFF_FAIL;
	}
}

// 在最后一行上显示 2018-02-28 18:31 （长度为16）
// 直接打印，在外面加flag让这个函数只跑一遍就可以，这个时间的显示用不着实时。
void time_disp_bottom(void){
	char disp1[17];
	// 格式如：2018-02-28
	disp1[0] = 48 + calendar.w_year/1000;
	disp1[1] = 48 + (calendar.w_year/100)%10;
	disp1[2] = 48 + (calendar.w_year/10)%10;
	disp1[3] = 48 + (calendar.w_year)%10;
	disp1[4] = '-';
	disp1[5] = 48 + (calendar.w_month)/10;
	disp1[6] = 48 + (calendar.w_month)%10;
	disp1[7] = '-';
	disp1[8] = 48 + (calendar.w_date)/10;
	disp1[9] = 48 + (calendar.w_date)%10;
	disp1[10] = ' ';
	// 18:31
	disp1[11] = 48 + (calendar.hour)/10;
	disp1[12] = 48 + (calendar.hour)%10;
	disp1[13] = ':';
	disp1[14] = 48 + (calendar.min)/10;
	disp1[15] = 48 + (calendar.min)%10;
	// 插入结束符
	disp1[16] = 0;

	// 打印在 OLED 上
	Disp_sentence_singleline(0,6,disp1,1);
}

// 在屏幕中间显示时间，如 18:31（超级大写）
u8 time_disp_big_flag=0;
void time_disp_big(void){
	char disp1[4];
	// 闪动中间的两个圆点
	if (timenew){
		disp1[0] = (calendar.hour)/10;
		disp1[1] = (calendar.hour)%10;
		disp1[2] = (calendar.min)/10;
		disp1[3] = (calendar.min)%10;

		// 打印数字
		show_time_big(4, 1, disp1[0]);
		show_time_big(28, 1, disp1[1]);
		show_time_big(76, 1, disp1[2]);
		show_time_big(100, 1, disp1[3]);

		if (time_disp_big_flag == 0){
			show_time_pot();
			time_disp_big_flag = 1;
		}
		else {
			close_time_pot();
			time_disp_big_flag = 0;
		}
	}
	timenew=0;
}

// 按键图标显示
void key_disp(uint8_t x,uint8_t y,uint8_t key,uint8_t clr){
	//clr 0表示不清空 1表示清空
	char disp[2];
	if (key == 0xa)
		disp[0] = '*';
	else if (key == 0xb)
		disp[0] = '#';
	else
		disp[0] = key+48;
	disp[1] = 0;//字符串结尾
	Disp_sentence(x,y,disp,clr);
}

// 用户编号采集函数，一个交互界面，用户设置编号
uint8_t user_id_cap(uint32_t * user_id,user_type * user_type_o){
	//user_id 采集到的id
	//user_type_o 这个id对应的用户类型，如果是空id，则这个变量不用管
	//采集用户输入的编号串
	*user_type_o = admin;
	int i=0;
	uint8_t key;
	uint8_t arr[3]={0,0,0};//数组存储用户输入的编号
	MY_USER my_user1;
	uint32_t addr;
	Disp_sentence(20,2,"请输入编号",1);
	OLED_Show_inverse(40,4,'0');
	OLED_Show(48,4,'0',0,1);
	OLED_Show(56,4,'0',0,1);
	*user_id = 0;
	while(1)
	{
		key = Wait_Key();
		if (key == NO_KEY)
			return USER_LOGIN_TIMEOUT;
		else if (key==0x0a)//星号
		{
			if(i==0)
				return USER_BACK;
			else
				i--;
		}
		else if (key==0x0b)//确认
		{
			//判断用户id是否重复
			for (i=0;i<MY_USER_MAX_NUM;i++)
			{
				addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
				STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2);
				if (my_user1.flag == 0xAA)
				{
					if (my_user1.number ==  *user_id)
					{
						*user_type_o = my_user1.my_user_type;
						 return USER_ID_REDUP;
					}
				}
			}
			return USER_CONFIRM;
		}
		else //用户输入数字，更新arr
		{
			arr[i] = key;
			*user_id = 100*arr[0]+10*arr[1]+arr[2];
			if(i<2)//光标不在最右边，光标右移
				i++;
		}
		//重新绘制界面
		if(i==0)
		{
			OLED_Show_inverse(40,4,arr[0]+48);
			OLED_Show(48,4,arr[1]+48,0,1);
			OLED_Show(56,4,arr[2]+48,0,1);
		}
		else if (i==1)
		{
			OLED_Show(40,4,arr[0]+48,0,1);
			OLED_Show_inverse(48,4,arr[1]+48);
			OLED_Show(56,4,arr[2]+48,0,1);
		}
		else
		{
			OLED_Show(40,4,arr[0]+48,0,1);
			OLED_Show(48,4,arr[1]+48,0,1);
			OLED_Show_inverse(56,4,arr[2]+48);
		}

	}
}

// 恢复出厂设置
void back2factory(void){
	int i;
	QS808_CMD_DEL_ALL();//删除全部指纹
	FLASH_Unlock();
	for (i=48;i<64;i++)
		FLASH_ErasePage(i*1024+STM32_FLASH_BASE);//擦除用户数据扇区
	FLASH_Lock();//上锁
}

// // 时间显示，旧版本的函数，现在已经不再使用
// void time_disp(void){
// 	// int m,n;
// 	char disp1[15];
// 	char disp2[9];
// 	if (timenew){
// 		disp1[0] = 48 + calendar.w_year/1000;
// 		disp1[1] = 48 + (calendar.w_year/100)%10;
// 		disp1[2] = 48 + (calendar.w_year/10)%10;
// 		disp1[3] = 48 + (calendar.w_year)%10;
// 		disp1[4] = '年'>>8;
// 		disp1[5] = '年';
// 		disp1[6] = 48 + (calendar.w_month)/10;
// 		disp1[7] = 48 + (calendar.w_month)%10;
// 		disp1[8] = '月'>>8;
// 		disp1[9] = '月';
// 		disp1[10] = 48 + (calendar.w_date)/10;
// 		disp1[11] = 48 + (calendar.w_date)%10;
// 		disp1[12] = '日'>>8;
// 		disp1[13] = '日';
// 		disp1[14] = 0;//结束
// 		disp2[0] = 48 + (calendar.hour)/10;
// 		disp2[1] = 48 + (calendar.hour)%10;
// 		disp2[2] = ':';
// 		disp2[3] = 48 + (calendar.min)/10;
// 		disp2[4] = 48 + (calendar.min)%10;
// 		disp2[5] =  ':';
// 		disp2[6] = 48 + (calendar.sec)/10;
// 		disp2[7] = 48 + (calendar.sec)%10;
// 		disp2[8] = 0;//结束
// 		Disp_sentence(2,2,disp1,0);
// 		Disp_sentence(25,4,disp2,0);
// 		Disp_sentence(30,6,"星期",0);
// 		switch(calendar.week){
// 			case 1:Disp_sentence(62,6,"一",0);break;
// 			case 2:Disp_sentence(62,6,"二",0);break;
// 			case 3:Disp_sentence(62,6,"三",0);break;
// 			case 4:Disp_sentence(62,6,"四",0);break;
// 			case 5:Disp_sentence(62,6,"五",0);break;
// 			case 6:Disp_sentence(62,6,"六",0);break;
// 			case 0:Disp_sentence(62,6,"日",0);break;
// 			default:Disp_sentence(62,6,"??",0);break;
// 		}
// 		timenew = 0;
// 		//显示电量
// 		if (Battery_quantity<3.7)
// 			OLED_Show_Power(0);
// 		else if (Battery_quantity<4.2)
// 			OLED_Show_Power(1);
// 		else if (Battery_quantity<4.7)
// 			OLED_Show_Power(2);
// 		else if (Battery_quantity<5.2)
// 			OLED_Show_Power(3);
// 		else
// 			OLED_Show_Power(4);
// 	}
// }
