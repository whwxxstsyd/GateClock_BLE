#include "./my_board.h"


extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 USART1_RecvBuf_Length;


int main(void) {
	delay_init();			// 【系统时钟】初始化
	RTC_Init();				// 【RTC时钟】初始化
	TSM12_Init();			// 【触摸按键芯片】初始化
	BLE_init();				// 蓝牙初始化
	debug_usart_init();		// 串口初始化
	power_ctrl_init();		// 【电源控制】初始化
	Power_ctrl_on();		// 打开电源控制
	RC522_Init();			// 【射频卡芯片】初始化
	TIM3_Int_Init(99,7199);	// 定时器3，测试函数运行时间用,10K频率计数,每10ms一个中断
	QS808_Init();			// 【指纹采集头】初始化
	Gate_Init();			// 【门锁机械控制】初始化
	OLED_Init();			// 【OLED】初始化
	// delay_ms(100);
	// VCC_Adc_Init();			// 【ADC】通道初始化
	UT588C_init();			// 【语音芯片】初始化
	QS808_CMD_DEL_ALL();	// 删除全部指纹

	u16 temp_cmdid,temp_userid,temp_return;
	u32 temp_RFCARD_ID;
	while(1) {
		// show_clock_close_big();
		// 如果接收到了数据传入，说明手机端发来了信息，可能要进行信息录入或者一键开锁
		if ( Usart_RecvOrder(USART1)==SYS_RECV_ORDER ) {
			// 根据 temp_cmdid 来进行分支判断
			temp_cmdid = RecvBuf2Cmdid();
			switch( temp_cmdid ) {
				/************************* 接收到【添加指纹】指令 *************************/
				case CMDID_ADD_FINGER:
					SPEAK_DUDUDU();
					if(Add_Finger(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_ADD_Success(USART1, 3, temp_userid);
					// 延时一段时间，防止直接进入指纹解锁步骤
					delay_ms(1000);
					break;


				/************************* 接收到【删除指纹】指令 *************************/
				case CMDID_DEL_FINGER:
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid(); 
					if (Delete_Finger(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_DEL_Success(USART1);
					else
						Usart_SendFinger_DEL_Error(USART1);
					break;


				/************************ 接收到【添加射频卡】指令 ************************/
				case CMDID_ADD_RFCARD:
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
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_RFCard(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendRFCard_DEL_Success(USART1);
					else
						Usart_SendRFCard_DEL_Error(USART1);
					break;


				/************************* 接收到【添加密码】指令 *************************/
				case CMDID_ADD_PASSWORD:
					SPEAK_DUDUDU();
					temp_return = Add_Password(&temp_userid);
					if(temp_return== ERROR_CODE_SUCCESS)
						Usart_SendPassword_ADD_Success(USART1, temp_userid);
					else
						Usart_SendPassword_ADD_Error(USART1, temp_return);
					break;


				/************************* 接收到【删除密码】指令 *************************/
				case CMDID_DEL_PASSWORD:
					SPEAK_DUDUDU();
					temp_userid = RecvBuf2Userid();
					if (Delete_Password(temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendPassword_DEL_Success(USART1);
					else
						Usart_SendPassword_DEL_Error(USART1);
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
				temp_return = Confirm_Finger();
				if (temp_return==ERROR_CODE_SUCCESS)	SPEAK_OPEN_THE_DOOR();
				else									SPEAK_DUDUDU();
				// 防止短时间内再次进入指纹检测
				delay_ms(1000);
			}
			
			// 如果检测到有射频卡靠近，就开始检测射频卡的正确性，准备开门
			if (RFCard_test(&temp_RFCARD_ID) == RFCARD_DETECED) {
				temp_return = Confirm_RFCard(temp_RFCARD_ID);
				if (temp_return==ERROR_CODE_SUCCESS)	SPEAK_OPEN_THE_DOOR();
				else									SPEAK_DUDUDU();
				// 防止短时间内再次进入射频卡检测
				delay_ms(1000);
			}

			// 如果检测到有按键按下，就进入密码解锁界面
			if (TMS12_ReadOnKey() != KEY_NULL) {
				u8 password_buf[LENGTH_KEY_BUF], buf_length=0, last_press, temp;
				SPEAK_DUDUDU();
				Disp_sentence(24,0,"请输入密码",1);
				Create_NewPasswordBuf(password_buf);
				while(1) {
					// 更新按键缓冲区
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
						}
						else {
							show_clock_close_big();
						}
						OLED_CLS();
						break;
					}
					else {
						OLED_CLS();
						break;
					}
				}
			}
		}
	}
}
