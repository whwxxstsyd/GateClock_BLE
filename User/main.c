#include "./my_board.h"


extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 USART1_RecvBuf_Length;
extern u8 WAKEUP_SOURCE;
extern QS808_Rec_Buf_type QS808_Rec_Buf;
extern _calendar_obj calendar;
extern u8 BLE_MAC[17];

float Battery_quantity;
u8 SLEEP_MAX = 30;


void Interface(void);


int main(void) {
	delay_init();			// 【系统时钟】初始化
	RTC_Init();				// 【RTC时钟】初始化
	TSM12_Init();			// 【触摸按键芯片】初始化
	BLE_init();				// 【蓝牙】初始化
	debug_usart_init();		// 【串口】初始化
	power_ctrl_init();		// 【电源控制】初始化
	Power_ctrl_on();		// 打开电源控制
	RC522_Init();			// 【射频卡芯片】初始化
	TIM3_Int_Init(99,7199);	// 定时器3，测试函数运行时间用,10K频率计数,每10ms一个中断
	QS808_Init();			// 【指纹采集头】初始化
	Gate_Init();			// 【门锁机械控制】初始化
	OLED_Init();			// 【OLED】初始化
	delay_ms(100);
	VCC_Adc_Init();			// 【ADC】通道初始化
	UT588C_init();			// 【语音芯片】初始化
	NewLed_Init();			// 【按键灯】初始化
	Init_BLE_MAC();			// 【蓝牙MAC】地址初始化
	// QS808_CMD_DEL_ALL();	// 删除全部指纹


	u8 work_flag;
	u16 temp_cmdid, temp_userid, temp_return, sleep_count=0;
	u32 temp_RFCARD_ID;
	Interface();
	LED_OFF2ON();


	while(1) {
		// 睡眠计数++
		// sleep_count++;

		// 显示一次主界面
		if (work_flag==1) {
			work_flag = 0;
			Interface();
		}

		// 判断是否该进入睡眠模式
		if (sleep_count>=SLEEP_MAX && QS808_Rec_Buf.Trans_state==reset) {
			sleep_count = 0;
			QS808_Rec_Buf_refresh();
			Disp_sentence(48,2,"休眠",1);
			delay_ms(500);
			PWR_Standby_Mode();

			// 唤醒之后从这里开始执行程序，先执行一次指纹扫描，加速开锁进程
			LED_OFF2ON();

			// 如果是指纹头唤醒
			if(WAKEUP_SOURCE==0) {
				// 如果检测到有手指按下，就开始检测指纹正确性，准备开门
				if (QS808_CMD_FINGER_DETECT()==ERR_FINGER_DETECT) {
					sleep_count = SLEEP_MAX;
					temp_return = Confirm_Finger();
					if (temp_return==ERROR_CODE_SUCCESS) {
						show_clock_open_big();
						SPEAK_OPEN_THE_DOOR();
						Gate_Unlock();
					}
					else {
						Disp_sentence(28,2,"验证失败",1);
						SPEAK_OPEN_THE_DOOR_FAIL();
						LED_OpenError();
						delay_ms(800);	// 验证失败的显示时间
					}
				}
			}
			else {
				Interface();
			}
		}

		// 如果接收到了数据传入，说明手机端发来了信息，可能要进行信息录入或者一键开锁
		if ( Usart_RecvOrder(USART1)==SYS_RECV_ORDER ) {
			// 根据 temp_cmdid 来进行分支判断
			temp_cmdid = RecvBuf2Cmdid();
			switch( temp_cmdid ) {
				/************************* 接收到【更新时间】指令 *************************/
				case CMDID_SET_TIME:
					sleep_count = 0;
					SPEAK_DUDUDU();
					Cogradient_Time();
					break;

				/************************* 接收到【添加指纹】指令 *************************/
				case CMDID_ADD_FINGER:
					sleep_count = 0;
					SPEAK_DUDUDU();
					if(Add_Finger(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_ADD_Success(USART1, 3, temp_userid);
					// 延时一段时间，防止直接进入指纹解锁步骤
					delay_ms(1000);
					break;


				/************************* 接收到【删除指纹】指令 *************************/
				case CMDID_DEL_FINGER:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_Finger(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_DEL_Success(USART1);
					else
						Usart_SendFinger_DEL_Error(USART1);
					break;


				/************************ 接收到【添加射频卡】指令 ************************/
				case CMDID_ADD_RFCARD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_return = Add_RFCard(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendRFCard_ADD_Success(USART1, temp_userid);
					else
						Usart_SendRFCard_ADD_Error(USART1, temp_return);
					// 延时一段时间，防止直接进入射频卡解锁步骤
					delay_ms(1000);
					break;


				/************************ 接收到【删除射频卡】指令 ************************/
				case CMDID_DEL_RFCARD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_RFCard(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendRFCard_DEL_Success(USART1);
					else
						Usart_SendRFCard_DEL_Error(USART1);
					break;


				/************************* 接收到【添加密码】指令 *************************/
				case CMDID_ADD_PASSWORD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_return = Add_Password(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendPassword_ADD_Success(USART1, temp_userid);
					else
						Usart_SendPassword_ADD_Error(USART1, temp_return);
					break;


				/************************* 接收到【删除密码】指令 *************************/
				case CMDID_DEL_PASSWORD:
					sleep_count = 0;
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_Password(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendPassword_DEL_Success(USART1);
					else
						Usart_SendPassword_DEL_Error(USART1);
					break;
				

				/************************* 接收到【一键开锁】指令 *************************/
				case CMDID_OPEN_DOOR:
					sleep_count = 0;
					show_clock_open_big();
					Gate_Unlock();
					SPEAK_OPEN_THE_DOOR();
					// 一般开锁都是成功的，也没有办法检测开锁是否成功。。所以直接返回开锁成功就是了
					Usart_SendOpenDoor_Success(USART1);
					break;


				/************************************************************************/
				default:
					break;
			}
		}

		// 如果没有接收到手机端发来的信息，那就时刻准备开锁
		else {
			// 如果检测到有手指按下，就开始检测指纹正确性，准备开门
			if (QS808_CMD_FINGER_DETECT()==ERR_FINGER_DETECT) {
				sleep_count = 0;
				work_flag = 1;
				temp_return = Confirm_Finger();
				if (temp_return==ERROR_CODE_SUCCESS) {
					show_clock_open_big();
					SPEAK_OPEN_THE_DOOR();
					Gate_Unlock();
				}
				else {
					Disp_sentence(28,2,"验证失败",1);
					LED_OpenError();
					SPEAK_OPEN_THE_DOOR_FAIL();
				}
				// 防止短时间内再次进入指纹检测
				delay_ms(800);
			}

			// 如果检测到有射频卡靠近，就开始检测射频卡的正确性，准备开门
			if (RFCard_test(&temp_RFCARD_ID) == RFCARD_DETECED) {
				sleep_count = 0;
				work_flag = 1;
				temp_return = Confirm_RFCard(temp_RFCARD_ID);
				if (temp_return==ERROR_CODE_SUCCESS) {
					show_clock_open_big();
					SPEAK_OPEN_THE_DOOR();
					Gate_Unlock();
				}
				else {
					Disp_sentence(28,2,"验证失败",1);
					SPEAK_OPEN_THE_DOOR_FAIL();
					LED_OpenError();
				}
				// 验证失败显示时间
				delay_ms(800);
			}

			// 如果检测到有按键按下，就进入密码解锁界面，准备开门
			if (TMS12_ReadOnKey() != KEY_NULL) {
				sleep_count = 0;
				work_flag = 1;
				u8 password_buf[LENGTH_KEY_BUF], buf_length=0, last_press, temp;
				SPEAK_DUDUDU();
				OLED_CLS();

				// 密码输入界面
				Battery_quantity = Get_Battery();
				if (Battery_quantity<3.7)		OLED_Show_Power(0);
				else if (Battery_quantity<4.2)	OLED_Show_Power(1);
				else if (Battery_quantity<4.7)	OLED_Show_Power(2);
				else if (Battery_quantity<5.2)	OLED_Show_Power(3);
				else 							OLED_Show_Power(4);
				Disp_sentence(24,2,"请输入密码",0);

				Create_NewPasswordBuf(password_buf);
				while(1) {	
					// 更新按键缓冲区X
					temp = Update_KeyBuf(password_buf, &buf_length, &last_press);
					// 如果现在正在输入密码，就响一下
					if (temp==PASSWORD_CODE_INPUTTING) {
						SPEAK_DUDUDU();
						Interface_Password(buf_length);
					}
					// 如果密码输入超时或者输入退出，就直接break
					else if (temp==PASSWORD_CODE_TIMEOUT || temp==PASSWORD_CODE_QUIT) {
						SPEAK_DUDUDU();
						OLED_CLS();
						break;
					}
					// 如果密码输入完成，就开始验证密码的准确性
					else if (temp==PASSWORD_CODE_COMPLETE) {
						SPEAK_DUDUDU();
						if (Confirm_Password(password_buf, buf_length)==ERROR_CODE_SUCCESS) {
							show_clock_open_big();
							SPEAK_OPEN_THE_DOOR();
							Gate_Unlock();
						}
						else {
							Disp_sentence(28,2,"验证失败",1);
							SPEAK_OPEN_THE_DOOR_FAIL();
							LED_OpenError();
						}
						// 留给界面显示
						delay_ms(800);
						break;
					}
					// 密码长度不足6位
					else if (temp==PASSWORD_CODE_SHORT) {
						SPEAK_DUDUDU();
						Disp_sentence(28,2,"验证失败",1);
						SPEAK_OPEN_THE_DOOR_FAIL();
						LED_OpenError();
						// 留给界面显示
						delay_ms(800);
						break;
					}
				}
			}
		}
	}
}

// 唤醒之后显示的人机交互界面
void Interface(void) {
	char disp1[11];

	OLED_CLS();

	// 第一行显示【电池电量】
	Battery_quantity = Get_Battery();
	if (Battery_quantity<3.7)		OLED_Show_Power(0);
	else if (Battery_quantity<4.2)	OLED_Show_Power(1);
	else if (Battery_quantity<4.7)	OLED_Show_Power(2);
	else if (Battery_quantity<5.2)	OLED_Show_Power(3);
	else 							OLED_Show_Power(4);

	// 如果缺电那就直接显示【请更换电池】
	if (Battery_quantity<=4.2) {
		Disp_sentence(23,2,"请更换电池",0);
	}
	// 如果不缺电，那就显示【欢迎使用】。并且显示时间信息
	else {
		Disp_sentence(32,2,"欢迎使用",0);

		// 第三行显示年月，格式如：2018-02-28
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
		disp1[10] = 0;	// 插入结束符
		Disp_sentence_singleline(23,4,disp1,0);

		// 第四行显示小时和分钟，18:31
		disp1[0] = 48 + (calendar.hour)/10;
		disp1[1] = 48 + (calendar.hour)%10;
		disp1[2] = ':';
		disp1[3] = 48 + (calendar.min)/10;
		disp1[4] = 48 + (calendar.min)%10;
		disp1[5] = 0;	// 插入结束符
		Disp_sentence_singleline(43,6,disp1,0);
	}
}






#if 0

// 返回 无重复字符的最长字串
int lengthOfLongestSubstring(char* s) {
    int maxlen = 0, s_len = 0, s_start = 0, index = 0;
    if (NULL == s) return;

    while (s[index] != '\0') {
        // search for char in window
        int i;
        for (i = s_start; i < index; i++) {
            if (s[i] == s[index]) {
                if (s_len > maxlen)
                    maxlen = s_len;
                    s_start = i+1;
                    s_len = index - s_start;
                break;
            }
        }
        s_len++;
        index++;
    }
    if (s_len > maxlen) maxlen = s_len;
    return maxlen;
}

// 两个排序数组的中位数
double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
	float temp[nums1Size+nums2Size];
    // 奇数取中间
	if ((nums1Size+nums2Size)%2 == 1) {
		int a=0,b=0;
		for (int i=0; i<(nums1Size+nums2Size); i++) {
			if ( (a<nums1Size)&&(nums1[a]<=nums2[b] || b==nums2Size) ) {
				temp[i] = nums1[a];
				a++;
			}
			else {
				temp[i] = nums2[b];
				b++;
			}
		}
		return temp[(nums1Size+nums2Size)/2];
	}
	// 偶数求平均
	else {
		int a=0,b=0;
		for (int i=0; i<(nums1Size+nums2Size); i++) {
			if ( (a<nums1Size)&&(nums1[a]<=nums2[b] || b==nums2Size) ) {
				temp[i] = nums1[a];
				a++;
			}
			else {
				temp[i] = nums2[b];
				b++;
			}
		}
		return (temp[(nums1Size+nums2Size)/2]+temp[(nums1Size+nums2Size)/2-1])/2;
	}
}

// 最长回文子串（回文是指正反读都一样）。s的长度最大为1000
char* longestPalindrome(char* s) {
    int length=0, i=0, j=0, max_length=0, temp_length=0, start=0;

	// 获取字符串长度
	while(1) {
		if (s[i++]!='\0') length++;
		else break;
	}

	char result[1001];
	char jyz[4];

	// 制作反字符串
	char temps[1000*3];
	for (i=0; i<length; i++) {
		temps[i+length] = s[length-i-1];
	}
	for (i=0; i<length; i++) {
		temps[i] = '\0';
	}
	for (i=length*2; i<length*3; i++) {
		temps[i] = '\0';
	}

	// 查找反字符串和原始字符串的最大相同子串
	for (i=length*2+1; i>0; i--) {
		// 制作窗数组
		char *window = &temps[i-1];

		temp_length = 0;
		for (j=0; j<length; j++) {
			if (s[j]==window[j]) {
				if (temp_length==0)	start = j;
				temp_length++;
			}
		}
		if (temp_length>max_length)	{
			max_length = temp_length;
			for (j=0; j<max_length; j++) {
				result[j] = s[start+j];
			}
			for (j=temp_length; j<1001; j++) {
                result[temp_length] = '\0';
            }
		}
	}

	// 网站有问题，我这样直接输出都说我 NULL  惊呆了也是。
	for (i=0; i<4; i++) {
		jyz[i] = 'a';
	}

	return jyz;
}

// 最长回文子串（回文是指正反读都一样）。s的长度最大为1000
// Manacher算法专治回文问题的各种不服
char* longestPalindrome(char* s) {

}
// 马拉车算法
int Manacher() {
    int len = Init();  //取得新字符串长度并完成向s_new的转换
    int maxLen = -1;   //最长回文长度
    int id;
    int mx = 0;

    for (int i = 1; i < len; i++) {
        if (i<mx)	p[i] = min(p[2*id-i], mx-i);//需搞清楚上面那张图含义, mx和2*id-i的含义
        else		p[i] = 1;

        while (s_new[i-p[i]] == s_new[i+p[i]])	//不需边界判断，因为左有'$',右有'\0'
            p[i]++;

		// 我们每走一步i，都要和mx比较，我们希望mx尽可能的远，这样才能更有机会执行if(i < mx)这句代码，从而提高效率
        if (mx < i + p[i]) {
            id = i;
            mx = i + p[i];
        }
        maxLen = max(maxLen, p[i] - 1);
    }
    return maxLen;
}

/**
 * 
 * author 刘毅（Limer）
 * date   2017-02-25
 * mode   C++ 
 */
#include<iostream>  
#include<string.h>
#include<algorithm>  
using namespace std;

char s[1000];
char s_new[2000];
int p[2000];

int Init()
{
    int len = strlen(s);
    s_new[0] = '$';
    s_new[1] = '#';
    int j = 2;

    for (int i = 0; i < len; i++)
    {
        s_new[j++] = s[i];
        s_new[j++] = '#';
    }

    s_new[j] = '\0';  //别忘了哦  

    return j;  //返回s_new的长度  s
}

int Manacher()
{
    int len = Init();  //取得新字符串长度并完成向s_new的转换  
    int maxLen = -1;   //最长回文长度  

    int id;
    int mx = 0;

    for (int i = 1; i < len; i++)
    {
        if (i < mx)
            p[i] = min(p[2 * id - i], mx - i);  //需搞清楚上面那张图含义, mx和2*id-i的含义
        else
            p[i] = 1;

        while (s_new[i - p[i]] == s_new[i + p[i]])  //不需边界判断，因为左有'$',右有'\0'  
            p[i]++;


        if (mx < i + p[i])  //我们每走一步i，都要和mx比较，我们希望mx尽可能的远，这样才能	更有机会执行if (i < mx)这句代码，从而提高效率  
        {
            id = i;
            mx = i + p[i];
        }

        maxLen = max(maxLen, p[i] - 1);
    }

    return maxLen;
}

int main()
{
    while (printf("请输入字符串：\n"))
    {
        scanf("%s", s);
        printf("最长回文长度为 %d\n\n", Manacher());
    }

    return 0;
}


#endif
