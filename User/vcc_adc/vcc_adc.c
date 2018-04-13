#include "my_board.h"
#include "vcc_adc.h"
// ADC通道初始化
void VCC_Adc_Init(void) {
	// io初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	VCC_ADC_GPIO_CLK, ENABLE );

	GPIO_InitStructure.GPIO_Pin = VCC_ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(VCC_ADC_GPIO_PORT, &GPIO_InitStructure);
	ADC_InitTypeDef ADC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, ENABLE );	// 使能GPIOA,ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div8);	// 分频因子

	ADC_DeInit(ADC1);  //将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	// ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;		// 模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	// 模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	// 转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	// ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	// 顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);		// 根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器

	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

	ADC_ResetCalibration(ADC1);	//重置指定的ADC1的复位寄存器

	while(ADC_GetResetCalibrationStatus(ADC1));	//获取ADC1重置校准寄存器的状态,设置状态则等待

	ADC_StartCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1));	// 获取指定ADC1的校准程序,设置状态则等待
}

void VCC_Adc_Sleep(void) {
	ADC_Cmd(ADC1, DISABLE);

	ADC_TempSensorVrefintCmd(DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );
}

u16 VCC_Get_Adc(u8 ch) {
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道3,第一个转换,采样时间为239.5周期
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

// 获取通道ch的转换值
// 取times次,然后平均
u16 VCC_Get_Adc_Average(u8 ch,u8 times) {
	u32 temp_val=0;
	u8 t;
	for (t=0;t<times;t++) {
		temp_val+=VCC_Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
}

// 获取电池电量
float Get_Battery(void) {
	u32 adcx,adcx2;
	float charge;
	adcx=VCC_Get_Adc_Average(ADC_Channel_Vrefint,20);	//读取通道16,20次取平均
	adcx2 = VCC_Get_Adc_Average(ADC_Channel_0,20); // 外部电压
	charge = 1.2/adcx*adcx2 * 2;
	charge = 1.5799f*charge - 1.8433f;//测量值与真值成一次关系，MATLAB拟合得到的

	return charge;
}

// 低电量报警
u16 Battery_Alarm(void) {
	
	return 0;
}
