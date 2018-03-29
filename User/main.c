#include "my_board.h"
#include "./data/data_def.h"
#include "./lock_ctrl/lock_ctrl.h"
#include "./OLED/oled.h"
#include "./vcc_adc/vcc_adc.h"
#include "./dac/dac.h"
#include "./TSM12/TSM12.h"
// #include "./BLE/BLE.h"

extern _calendar_obj calendar;//时钟结构体
extern uint32_t ms10_cnt;
float Battery_quantity;


int main(void)
{
	delay_init();			// 【系统时钟】初始化
	RTC_Init();				// 【RTC时钟】初始化
	TIM3_Int_Init(99,7199);	// 定时器3，测试函数运行时间用,10K频率计数,每10ms一个中断


	power_ctrl_init();		// 【电源控制】初始化
	Power_ctrl_on();		// 打开电源控制
	OLED_Init();			// 【OLED】初始化
	QS808_Init();			// 【指纹采集头】初始化
	TSM12_Init();			// 【触摸按键芯片】初始化
	RC522_Init();			// 【射频卡芯片】初始化
	Gate_Init();			// 【门锁机械控制】初始化
	delay_ms(100);
	VCC_Adc_Init();			// 【ADC】通道初始化
	UT588C_init();			// 【语音芯片】初始化
	first_time_init();		// 上电初始化


	// BLE_init();				// 蓝牙初始化
	// debug_usart_init();		// 串口初始化


	// // 发送数据测试
	// u8 key;	
	// BleDataHead blue;
	// blue.m_magicCode = magicCode;
	// blue.m_version = version;
	// blue.m_totalLength = 12;
	// blue.m_cmdId = cmdId_IC;
	// blue.m_seq = 0x0000;
	// blue.m_errorCode = errorCode0;

	// int box = 11;
	// while(1){
	// 	// 扫描按键
	// 	key = IsKey();

	// 	// *
	// 	if (key==0x0a){
	// 		// Usart_SentMessage(UART_1, (u16*)&blue);
	// 		// Usart_SentMessage(UART_1, (u16*)&blue+1);
			
	// 		// for (u8 i=0; i<5; i++){
	// 		// 	pUsart_SentMessage(UART_1, (u16*)&blue+i);
	// 		// }

	// 		// for (u8 i=0; i<3; i++){
	// 		// 	pUsart_SendByte(UART_1, (u8*)&box+i);
	// 		// }
			
	// 		Usart_SendByte(UART_1, 0x31);
	// 		Usart_SendByte(UART_1, 0x32);
	// 		Usart_SendByte(UART_1, 0x33);


	// 		SPEAK_DUDUDU();
	// 	}
	// }

	// // 接收数据测试
	// while(1){
	// 	;
	// }


	// back2factory();


	// 界面控制部分
	start_interface();

}  // end main()
