#include "./keyboard/keyboard.h"
#include "./TSM12/TSM12.h"
#include "./Delay/delay.h"


uint8_t Key_Buffer[30];
extern float 	Battery_quantity;

// 返回当前按下的按键的键值
// return:	按键的键值。0~9；星号0x0a；井号0x0b；没有按下NO_KEY
u8 KeyScan(void) {
	u32 timeout_cnt;
	u8 key;

	// 检测是否有按键按下
	key = TMS12_ReadOnKey();

	// 如果没有按键按下，就直接 return NO_KEY
	if (key == KEY_NULL) {
		return NO_KEY;
	}

	// 如果有按键按下
	else {
		// 等待松手
		timeout_cnt = 0;
		while (TMS12_ReadOnKey()!=KEY_NULL){
			timeout_cnt++;
			if (timeout_cnt==100) {
				break;
			}
		}
		// 根据按下的按键，return 对应的键值
		switch (key) {
			case KEY_0:
				// printf("0\n");
				return 0x00;
			case KEY_1:
				// printf("1\n");
				return 0x01;
			case KEY_2:
				// printf("2\n");
				return 0x02;
			case KEY_3:
				// printf("3\n");
				return 0x03;
			case KEY_4:
				// printf("4\n");
				return 0x04;
			case KEY_5:
				// printf("5\n");
				return 0x05;
			case KEY_6:
				// printf("6\n");
				return 0x06;
			case KEY_7:
				// printf("7\n");
				return 0x07;
			case KEY_8:
				// printf("8\n");
				return 0x08;
			case KEY_9:
				// printf("9\n");
				return 0x09;
			case KEY_AST:
				// printf("*\n");
				return 0x0a;
			case KEY_POU:
				// printf("#\n");
				return 0x0b;
			default:
				return 100;
		}
	}
}


// // 缓存按键序列，这个函数只缓存6位密码，用于设置密码，因为只能设置6位
// uint8_t Key_Cap_6bits(uint8_t first_key, uint8_t* length,uint8_t disp_or_not) {
// 	int i,j,k;
// 	uint8_t key;
// 	*length = 0;
// 	char disp[7] = "      ";
// 	disp[0] = first_key + 48;
// 	if (disp_or_not)
// 		Disp_sentence(0,3,"*",1);
// 	else
// 		Disp_sentence(0,3,disp,1);
// 	for (i=0;i<30;i++) {
// 		Key_Buffer[i] = NO_KEY;
// 	}

// 	Disp_line(1,22,127,0);
// 	Disp_line(1,41,127,0);

// 	Key_Buffer[0] = first_key;
// 	// 这里i确实是从1开始，而不是0，没有问题,当i等于1时，Key_Buffer只保存了Key_Buffer[0],也就是说，此时只有一位密码
// 	for (i=1;i<7;i++) {
// 		key = Wait_Key();
// 		// 输入超时
// 		if(key == NO_KEY) {
// 			// 输入超时则清空按键buffer
// 			for (i=0;i<6;i++) {
// 				if(Key_Buffer[i] == NO_KEY)
// 					break;
// 				else
// 					Key_Buffer[i] = NO_KEY;
// 			}
// 			return KEY_TIMEOUT;
// 		}
// 		// 输入*取消
// 		else if(key == 0x0a) {
// 			if(i==0) {
// 				return KEY_CANCEL;
// 			}
// 			else {
// 				i=i-1;
// 				Key_Buffer[i] = NO_KEY;	// 删除上一位
// 				i=i-1;//i再-1 抵消for里面的i++
// 				// 显示部分
// 				// 从当前密码位向前数6位开始显示
// 				k = i-5;
// 				j = 0;
// 				while(j<6)
// 				{
// 					if (k>=0)
// 					{
// 						if (disp_or_not)
// 							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 						else
// 							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
// 						j++;
// 					}
// 					k++;
// 				}
// 			}
// 		}
// 		// 输入#确认
// 		else if (key == 0x0b) {
// 			*length = i;
// 			if(i<6) {
// 				Disp_sentence(2,2,"密码必须为6位",1);
// 				delay_ms(1000);
// 				OLED_CLS();
// 				Disp_line(1,22,127,0);
// 				Disp_line(1,41,127,0);
// 				i--;
// 			}
// 			else
// 				return KEY_CONFIRM;
// 		}
// 		// 将按键缓存进buffer
// 		else {
// 			// i==6说明已经输入够6位了，这时把i置为5，再次操作最后一位
// 			if(i==6) {
// 				i=5;
// 			}
// 			Key_Buffer[i] = key;
// 			// 显示部分，从当前密码位向前数8位开始显示
// 			k = i-5;
// 			j = 0;
// 			while(j<6) {
// 				if (k>=0) {
// 					if (disp_or_not)
// 						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 					else
// 						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
// 					j++;
// 				}
// 				k++;
// 			}
// 		}
// 		Disp_sentence(1,3,disp,0);
// 	}
// 	// 如果程序跑到这里说明密码太长（实际上是跑不到这里的）
// 	return KEY_TOO_LONG;
// }

// 【密码输入】界面
// first_key	是第一个按键，因为这个函数需要按键盘触发，所以第一个按键是在函数之外就发生的
// length		是按键序列的长度，不包括确认键
// disp_or_not	是明码选择；disp_or_not=0,显示输入的数字，用于登记；disp_or_not=1，不显示输入的数字，用于解锁和通过验证
// 序列捕捉到了全局变量 Key_Buffer
// 这个函数用于密码解锁时，因为此时可以输入很多位密码，而设置密码时，只能输入6位
// uint8_t Key_Cap(uint8_t first_key, uint8_t* length,uint8_t disp_or_not)
// {
// 	int i;
// 	int j;
// 	int k;
// 	uint8_t key;
// 	*length = 0;
// 	char disp[9] = "        ";
// 	disp[0] = first_key + 48;
// 	if (disp_or_not)
// 		Disp_sentence(0,3,"*",1);
// 	else
// 		Disp_sentence(0,3,disp,1);
// 	for (i=0;i<30;i++){
// 		Key_Buffer[i] = NO_KEY;
// 	}

// 	Disp_line(0,22,127,0);
// 	Disp_line(0,41,127,0);

// 	Key_Buffer[0] = first_key;

// 	// 这里i确实是从1开始，而不是0，没有问题,当i等于1时，Key_Buffer只保存了Key_Buffer[0],也就是说，此时只有一位密码
// 	for (i=1;i<30;i++){
// 		key = Wait_Key();
// 		// 输入超时
// 		if(key == NO_KEY){
// 			// 输入超时则清空按键buffer
// 			for (i=0;i<30;i++){
// 				if(Key_Buffer[i] == NO_KEY)
// 					break;
// 				else
// 					Key_Buffer[i] = NO_KEY;
// 			}
// 			return KEY_TIMEOUT;
// 		}
// 		// 输入*取消
// 		else if(key == 0x0a){
// 			if(i==0){
// 				return KEY_CANCEL;
// 			}
// 			else {
// 				i=i-1;
// 				Key_Buffer[i] = NO_KEY;//删除上一位
// 				i=i-1;//i再-1 抵消for里面的i++
// 				//显示部分
// 				k = i-7;//从当前密码位向前数8位开始显示
// 				j = 0;
// 				while(j<8){
// 					if (k>=0){
// 						if (disp_or_not)
// 							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 						else
// 							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
// 						j++;
// 					}
// 					k++;
// 				}
// 			}
// 		}
// 		// 输入#确认
// 		else if (key == 0x0b){
// 			Key_Buffer[i] = 0x0b;//确认位，用于密码检测
// 			*length = i;
// 			if(i==0){
// 				Disp_sentence(18,4,"密码不能为空",1);
// 				delay_ms(1000);
// 				OLED_CLS();
// 				Key_Buffer[i] = NO_KEY;//删除该位
// 				i=i-1;//抵消i++,重新录入first_key
// 			}
// 			else
// 				return KEY_CONFIRM;
// 		}
// 		// 将按键缓存进buffer
// 		else{
// 			Key_Buffer[i] = key;
// 			// 显示部分
// 			// 从当前密码位向前数8位开始显示
// 			k = i-7;
// 			j = 0;
// 			while(j<8){
// 				if (k>=0){
// 					if (disp_or_not)
// 						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 					else
// 						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
// 					j++;
// 				}
// 				k++;
// 			}
// 		}
// 		Disp_sentence(0,3,disp,0);
// 	}
// 	// 如果程序跑到这里，说明输入的按键串太长了
// 	return KEY_TOO_LONG;
// }

// //
// uint8_t start_interface_key_cap(uint8_t first_key, uint8_t* length,uint8_t disp_or_not){
// 	int i,j,k;
// 	uint8_t key;
// 	*length = 0;
// 	char disp[9] = "        ";
// 	disp[0] = first_key + 48;


// 	for (i=0;i<30;i++){
// 		Key_Buffer[i] = NO_KEY;
// 	}

// 	Key_Buffer[0] = first_key;

// 	// 这里i确实是从1开始，而不是0，没有问题,当i等于1时，Key_Buffer只保存了Key_Buffer[0],也就是说，此时只有一位密码
// 	for (i=1;i<30;i++){
// 		key = Wait_Key();
// 		// 输入超时
// 		if(key == NO_KEY){
// 			// 输入超时则清空按键buffer
// 			for (i=0;i<30;i++){
// 				if(Key_Buffer[i] == NO_KEY)
// 					break;
// 				else
// 					Key_Buffer[i] = NO_KEY;
// 			}
// 			return KEY_TIMEOUT;
// 		}
// 		// 输入*取消
// 		else if(key == 0x0a){
// 			if(i==0){
// 				return KEY_CANCEL;
// 			}
// 			else {
// 				i=i-1;
// 				Key_Buffer[i] = NO_KEY;//删除上一位
// 				i=i-1;//i再-1 抵消for里面的i++
// 				//显示部分
// 				k = i-7;//从当前密码位向前数8位开始显示
// 				j = 0;
// 				while(j<8){
// 					if (k>=0){
// 						if (disp_or_not)
// 							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 						else
// 							disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
// 						j++;
// 					}
// 					k++;
// 				}
// 			}
// 		}
// 		// 输入#确认
// 		else if (key == 0x0b){
// 			Key_Buffer[i] = 0x0b;//确认位，用于密码检测
// 			*length = i;
// 			if(i==0){
// 				Disp_sentence(16,4,"密码不能为空",1);
// 				delay_ms(1000);
// 				OLED_CLS();
// 				Key_Buffer[i] = NO_KEY;//删除该位
// 				i=i-1;//抵消i++,重新录入first_key
// 			}
// 			else
// 				return KEY_CONFIRM;
// 		}
// 		// 将按键缓存进buffer
// 		else{
// 			Key_Buffer[i] = key;
// 			// 显示部分
// 			// 从当前密码位向前数8位开始显示
// 			k = i-7;
// 			j = 0;
// 			while(j<8){
// 				if (k>=0){
// 					if (disp_or_not)
// 						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 					else
// 						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':Key_Buffer[k]+48;
// 					j++;
// 				}
// 				k++;
// 			}
// 		}
// 		Disp_sentence_singleline(0,4,disp,0);
// 	}
// 	//如果程序跑到这里，说明输入的按键串太长了
// 	return KEY_TOO_LONG;
// }

// // admin_verify3 密码输入小窗口
// uint8_t admin_verify3_key_cap(uint8_t first_key, uint8_t* length)
// //序列捕捉到了全局变量 Key_Buffer
// //这个函数用于密码解锁时，因为此时可以输入很多位密码，而设置密码时，只能输入6位
// {
// 	int i;
// 	int j;
// 	int k;
// 	uint8_t key;
// 	*length = 0;
// 	char disp[9] = "        ";
// 	disp[0] = first_key + 48;

// 	Disp_sentence(0,4,"*",0);

// 	for (i=0;i<30;i++){
// 		Key_Buffer[i] = NO_KEY;
// 	}


// 	Key_Buffer[0] = first_key;

// 	// 这里i确实是从1开始，而不是0，没有问题,当i等于1时，Key_Buffer只保存了Key_Buffer[0],也就是说，此时只有一位密码
// 	for (i=1;i<30;i++){
// 		key = Wait_Key();
// 		// 输入超时
// 		if(key == NO_KEY){
// 			// 输入超时则清空按键buffer
// 			for (i=0;i<30;i++){
// 				if(Key_Buffer[i] == NO_KEY)
// 					break;
// 				else
// 					Key_Buffer[i] = NO_KEY;
// 			}
// 			return KEY_TIMEOUT;
// 		}
// 		// 输入*取消
// 		else if(key == 0x0a){
// 			if(i==0){
// 				return KEY_CANCEL;
// 			}
// 			else {
// 				i=i-1;
// 				Key_Buffer[i] = NO_KEY;//删除上一位
// 				i=i-1;//i再-1 抵消for里面的i++
// 				//显示部分
// 				k = i-7;//从当前密码位向前数8位开始显示
// 				j = 0;
// 				while(j<8){
// 					if (k>=0){
// 						disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 						j++;
// 					}
// 					k++;
// 				}
// 			}
// 		}
// 		// 输入#确认
// 		else if (key == 0x0b){
// 			Key_Buffer[i] = 0x0b;//确认位，用于密码检测
// 			*length = i;
// 			if(i==0){
// 				Disp_sentence(18,4,"密码不能为空",1);
// 				delay_ms(1000);
// 				OLED_CLS();
// 				Key_Buffer[i] = NO_KEY;//删除该位
// 				i=i-1;//抵消i++,重新录入first_key
// 			}
// 			else
// 				return KEY_CONFIRM;
// 		}
// 		// 将按键缓存进buffer
// 		else{
// 			Key_Buffer[i] = key;
// 			// 显示部分
// 			// 从当前密码位向前数8位开始显示
// 			k = i-7;
// 			j = 0;
// 			while(j<8){
// 				if (k>=0){
// 					disp[j] = (Key_Buffer[k] == NO_KEY)?' ':'*';
// 					j++;
// 				}
// 				k++;
// 			}
// 		}
// 		Disp_sentence(0,4,disp,0);
// 	}
// 	//如果程序跑到这里，说明输入的按键串太长了
// 	return KEY_TOO_LONG;
// }

// 带等待的按键读取
uint8_t Wait_Key(void) {
	uint8_t key ;
	int cnt=0;
	do {
		key = KeyScan();
		cnt++;
	}while(key==NO_KEY && cnt<4000);
	return key;
}
