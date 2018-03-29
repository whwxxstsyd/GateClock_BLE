#include <stdio.h>
#include "./TSM12/TSM12.h"

// 【TSM12】电容触摸芯片

//PB13->SCL
//PB12->SDA
//初始化IIC
 
#define SDA_IN()  {GPIOB->CRH&=0XFFF0FFFF;GPIOB->CRH|=(u32)8<<16;}
#define SDA_OUT() {GPIOB->CRH&=0XFFF0FFFF;GPIOB->CRH|=(u32)3<<16;}

//IO操作函数	 
#define IIC_SCL    PBout(13) //SCL
#define IIC_SDA    PBout(12) //SDA	 
#define READ_SDA   PBin(12)  //输入SDA 

static void TSM12_PIN_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	TSM12_SDA_GPIO_CLK|TSM12_SCL_GPIO_CLK|TSM12_RST_GPIO_CLK|TSM12_I2CEN_GPIO_CLK, ENABLE );
	   
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = TSM12_SDA_PIN;
	GPIO_Init(TSM12_SDA_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(TSM12_SDA_GPIO_PORT,TSM12_SDA_PIN);//sda拉高

	GPIO_InitStructure.GPIO_Pin = TSM12_SCL_PIN;
	GPIO_Init(TSM12_SCL_GPIO_PORT, &GPIO_InitStructure);	
	GPIO_SetBits(TSM12_SCL_GPIO_PORT,TSM12_SCL_PIN);//scl拉高
	
	GPIO_InitStructure.GPIO_Pin = TSM12_RST_PIN;
	GPIO_Init(TSM12_RST_GPIO_PORT, &GPIO_InitStructure);	
	GPIO_ResetBits(TSM12_RST_GPIO_PORT,TSM12_RST_PIN);//复位拉低，不复位
	
	GPIO_InitStructure.GPIO_Pin = TSM12_I2CEN_PIN;
	GPIO_Init(TSM12_I2CEN_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(TSM12_I2CEN_GPIO_PORT,TSM12_I2CEN_PIN);//打开TSM12 i2c功能
	//中断引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
	GPIO_InitStructure.GPIO_Pin = TSM12_INT_PIN;
	GPIO_Init(TSM12_INT_GPIO_PORT, &GPIO_InitStructure);
	
	//中断初始化
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	
 	GPIO_EXTILineConfig(TSM12_INT_PIN_SOURCE_PORT,TSM12_INT_PIN_SOURCE_PIN);
	EXTI_InitStructure.EXTI_Line=TSM12_INT_EXT_LINE;	
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	 
		
	NVIC_InitStructure.NVIC_IRQChannel = TSM12_INT_IRQn;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x04;				
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure); 

}


//产生IIC起始信号
static void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(5);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(5);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
static void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(5);
	IIC_SCL=1; 
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(5);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;delay_us(5);	   
	IIC_SCL=1;delay_us(5);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
//			while(1)
//			{}
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
static void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(5);
	IIC_SCL=1;
	delay_us(5);
	IIC_SCL=0;
}
//不产生ACK应答		     
static void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(5);
	IIC_SCL=1;
	delay_us(5);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
static void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
    //IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			IIC_SDA=1;
		else
			IIC_SDA=0;
		txd<<=1; 	  
		delay_us(5);   
		IIC_SCL=1;
		delay_us(5); 
		IIC_SCL=0;	
		delay_us(5);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
static u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(5);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}



u16 TTP229_ReadOneByte(void)//读一个按键 
{		
	u8 high=0,low=0;
	u16 key_value=0;		  	    																 
	IIC_Start();  
	IIC_Send_Byte(0XAF);		//ttp259  的读地址，只接受读地址。 
	IIC_Wait_Ack();
	
	
  low=IIC_Read_Byte(1);
	high=IIC_Read_Byte(0);
	key_value=(high<<8)|low;		   	//十六键需要两个字节数据，八键只需要一个字节数据
	IIC_Stop();	    
	return key_value;
}

uint8_t TSM12_reg_write(uint8_t address,uint8_t value)
{
	IIC_Start();
	IIC_Send_Byte(0xD0);//写地址
	if(IIC_Wait_Ack())
		return 0x01;
	IIC_Send_Byte(address);//寄存器地址
	if(IIC_Wait_Ack())
		return 0x01;
	IIC_Send_Byte(value);//寄存器值
	if(IIC_Wait_Ack())
		return 0x01;
	IIC_Stop();
	return 0x00;	
}

uint8_t TSM12_reg_read(uint8_t address)//i2c通信失败 则直接返回0x00 模拟没有按下键盘
{
	uint8_t value;
	IIC_Start();
	IIC_Send_Byte(0xD0);//写地址
	if(IIC_Wait_Ack())
		return 0x00;
	IIC_Send_Byte(address);//寄存器地址
	if(IIC_Wait_Ack())
		return 0x00;
	IIC_Stop();
	
	IIC_Start();
	IIC_Send_Byte(0xD1);//读地址
	if(IIC_Wait_Ack())
		return 0x00;
	value = IIC_Read_Byte(0);
	IIC_Stop();
	return value;
	
}
void TSM12_SLEEP(void)
{
	//休眠 或者 关闭通道 都将导致 按键中断不能产生 很难用
	TSM12_reg_write(CTR1,0x23);
	TSM12_reg_write(CTR2,0x07);//打开休眠功能
//	TSM12_reg_write(Ch_hold1,0xff);//通道全部关闭
//	TSM12_reg_write(Ch_hold2,0x0f);
	GPIO_SetBits(TSM12_I2CEN_GPIO_PORT,TSM12_I2CEN_PIN);//关闭i2c功能	
}
void TSM12_Wakeup(void)
{
	GPIO_ResetBits(TSM12_I2CEN_GPIO_PORT,TSM12_I2CEN_PIN);//打开i2c
	TSM12_reg_write(CTR2,0x03);//关闭休眠功能
	TSM12_reg_write(CTR1,0x80);
}
void TSM12_Init(void)
{
	TSM12_PIN_Init();
	//rst
	delay_ms(100);
	GPIO_SetBits(TSM12_RST_GPIO_PORT,TSM12_RST_PIN);//复位
	delay_ms(100);
	GPIO_ResetBits(TSM12_RST_GPIO_PORT,TSM12_RST_PIN);//复位结束
	delay_ms(100);
	//reg config
	TSM12_reg_write(CTR2,0x0b);//软复位,但不休眠，这样反应速度快
	TSM12_reg_write(CTR2,0x07);	
	TSM12_reg_write(Sens1,0x88);//每4bit一个通道 高位表示感应范围 低三位表示灵敏度 数字越小越灵敏
	TSM12_reg_write(Sens2,0x88);
	TSM12_reg_write(Sens3,0x88);
	TSM12_reg_write(Sens4,0x88);
	TSM12_reg_write(Sens5,0x88);
	TSM12_reg_write(Sens6,0x88);	
	TSM12_reg_write(CTR1,0x80);//这个要配置成80，不然反应慢
	TSM12_reg_write(Ref_rst1,0x00);//重置参考值，这个好像可以重置一下，但之后要配置成0 不然出问题
	TSM12_reg_write(Ref_rst2,0x00);	
	TSM12_reg_write(Cal_hold1,0xff);//校准使能，配置看不出区别
	TSM12_reg_write(Cal_hold2,0x0f);	
	TSM12_reg_write(Ch_hold1,0x00);//通道使能，0是使能 1是失能
	TSM12_reg_write(Ch_hold2,0x00);
	
}

//键值表

uint8_t TMS12_ReadOnKey(void)
{
	uint8_t k1,k2,k3;
	uint32_t KEY;
	uint32_t mask=0x00000003;//掩码，取出KEY中按下的键值 
	uint8_t i=0;
	k1=TSM12_reg_read(Output1);
	k2=TSM12_reg_read(Output2);
	k3=TSM12_reg_read(Output3);
  KEY=(k1<<16)|(k2<<8)|(k3);
	for(i=0;i<12;i++)
	{
		if(KEY == mask)	
			return i;
		mask=mask<<2;
	}
	return 0xff;//0xff表示没有按下

}
