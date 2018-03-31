#include "./my_board.h"



// extern _calendar_obj calendar;//时钟结构体
// extern uint32_t ms10_cnt;

extern u8 USART_Recv_Flag;
extern u8 USART_RecvBuf[12];
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
	// OLED_Init();			// 【OLED】初始化
	// QS808_Init();			// 【指纹采集头】初始化
	// Gate_Init();			// 【门锁机械控制】初始化
	// delay_ms(100);
	// VCC_Adc_Init();			// 【ADC】通道初始化
	UT588C_init();			// 【语音芯片】初始化

	u16 cmdId,userNumber;
	
	while(1) {
		
		// 接收串口数据
		cmdId = Usart_RecvOrder(USART1);
		
		switch(cmdId) {
			case CMDID_NONE:
				break;
			case CMDID_RFCARD:
				SPEAK_DUDUDU();
				if(Add_RFCard(&userNumber) == ERROR_CODE_SUCCESS) {
					Usart_RFCard_Success(USART1, userNumber);
				}
				else {
					Usart_RFCard_Error(USART1);
				}
				break;
			default:
				break;
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
