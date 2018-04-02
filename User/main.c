#include "./my_board.h"



// extern _calendar_obj calendar;//时钟结构体
// extern uint32_t ms10_cnt;

extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[USART_RECVBUF_LENGTH];
extern u8 USART1_RecvBuf_Length;

// float Battery_quantity;

int main(void) {
	delay_init();			// 【系统时钟】初始化
	RTC_Init();				// 【RTC时钟】初始化
	// TSM12_Init();			// 【触摸按键芯片】初始化
	BLE_init();				// 蓝牙初始化
	debug_usart_init();		// 串口初始化
	power_ctrl_init();		// 【电源控制】初始化
	Power_ctrl_on();		// 打开电源控制
	RC522_Init();			// 【射频卡芯片】初始化
	TIM3_Int_Init(99,7199);	// 定时器3，测试函数运行时间用,10K频率计数,每10ms一个中断
	QS808_Init();			// 【指纹采集头】初始化
	// OLED_Init();			// 【OLED】初始化
	// Gate_Init();			// 【门锁机械控制】初始化
	// delay_ms(100);
	// VCC_Adc_Init();			// 【ADC】通道初始化
	UT588C_init();			// 【语音芯片】初始化
	QS808_CMD_DEL_ALL();	// 删除全部指纹


	u16 temp_cmdid,temp_userid;
	while(1) {
		// 如果接收到了数据传入
		if ( Usart_RecvOrder(USART1)==SYS_RECV_ORDER  ) {
			// 根据 temp_cmdid 来进行分支判断
			temp_cmdid = RecvBuf2Cmdid();
			switch( temp_cmdid ) {
				/************************ 接收到【添加射频卡】指令 ************************/
				case CMDID_ADD_RFCARD:
					SPEAK_DUDUDU();
					if(Add_RFCard(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendRFCard_ADD_Success(USART1, temp_userid);
					else
						Usart_SendRFCard_ADD_Error(USART1);
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


				/************************* 接收到【添加指纹】指令 *************************/
				case CMDID_ADD_FINGER:
					SPEAK_DUDUDU();
					if(Add_Finger(&temp_userid) == ERROR_CODE_SUCCESS)
						Usart_SendFinger_ADD_Success(USART1, 3, temp_userid);
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

				/************************************************************************/
				default:
					break;
			}
		}
	}
}





// if(USART_Recv_Flag==1) {
// 	USART_Recv_Flag = 0;
// 	i = 0;
// 	while(USART1_RecvBuf_Length--) {
// 		// 发送数据
// 		USART_SendData(USART1, USART_RecvBuf[i++]);

// 		// 等待发送结束
// 		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
// 	}
// 	USART1_RecvBuf_Length = 0;
// }
