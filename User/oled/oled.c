#include "./OLED/oled.h"
#include <stdlib.h>
#include "./OLED/FONT_TAB.h"
//sda   PA15
//scl   PB3
//IO方向设置
#define SDA_IN()  {GPIOA->CRH&=0X0FFFFFFF;GPIOA->CRH|=(u32)8<<28;}
#define SDA_OUT() {GPIOA->CRH&=0X0FFFFFFF;GPIOA->CRH|=(u32)3<<28;}

//IO操作函数
#define IIC_SCL    PBout(3) //SCL
#define IIC_SDA    PAout(15) //SDA
#define READ_SDA   PAin(15)  //输入SDA

static void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low
	delay_us(4);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据
}
static void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1;
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(4);
}
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入
	IIC_SDA=1;delay_us(1);
	IIC_SCL=1;delay_us(1);
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0
	return 0;
}
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
		delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL=1;
		delay_us(2);
		IIC_SCL=0;
		delay_us(2);
    }
	IIC_Wait_Ack();
}


static void I2C_Write_Data(u8 addr,u8 data)
{
    IIC_Start();
		IIC_Send_Byte(0x78);
    IIC_Send_Byte(addr);
		IIC_Send_Byte(data);
    IIC_Stop();
}

void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_Write_Data(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_Write_Data(0x40, I2C_Data);
}

void OLED_IIC_Init(void)//初始化,模拟II2C不使用引脚的复用。
{
	//屏幕部分
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(OLED_SCL_GPIO_CLK|OLED_SDA_GPIO_CLK, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

	GPIO_InitStructure.GPIO_Pin = OLED_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(OLED_SCL_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = OLED_SDA_PIN;
	GPIO_Init(OLED_SDA_GPIO_PORT, &GPIO_InitStructure);
	IIC_SDA=1;
	IIC_SCL=1;
}

void OLED_Init(void)
{
	OLED_IIC_Init();
	int i;
	for (i=0;i<=1000;i++);

	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	OLED_CLS();
	delay_ms(100);
}

// 设置起始点坐标
void OLED_SetPos(unsigned char x, unsigned char y){
	WriteCmd(0xb0+y);
	WriteCmd( ((x&0xf0)>>4)|0x10 );				//列的高地址
	WriteCmd( (x&0x0f)|0x00 );					//列的低地址
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);	//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}

void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void){
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}




void OLED_Show(unsigned char x, unsigned char y,char data1,char data2,uint8_t CNorEN)
/*显示单个字符，包括汉字，英文，数字，特殊字符
x:横坐标 0-127
y:纵坐标 0-7
data1 汉字的高位（或英文）
data2 汉字的低位（显示英文时，这个参数无效）
CNorEN: 0 汉字 1 英文，数字，特殊字符
*/
{
	unsigned char wm=0;
	OLED_SetPos(x , y);
	int addr = 0;
	int i=0;
	// 显示英文
	if (CNorEN){
		addr = (data1-32)*16;//-32是因为字库中前32个 不可显示字符都没有
		for(wm = 0;wm < 8;wm++)
		{
			WriteDat(F8X16[addr]);
			addr += 1;
		}
		OLED_SetPos(x,y+1);
		for(wm = 0;wm < 8;wm++)
		{
			WriteDat(F8X16[addr]);
			addr += 1;
		}
	}
	// 显示中文
	else {
		while(i<sizeof(HZ16x16))
		{
			if((HZ16x16[i] == data1) && (HZ16x16[i+1] == data2))
			{
				addr = i+2;
				break;
			}
			i = i+34;
		}

		for(wm = 0;wm < 16;wm++)
		{
			WriteDat(HZ16x16[addr]);
			addr += 1;
		}
		OLED_SetPos(x,y+1);
		for(wm = 0;wm < 16;wm++)
		{
			WriteDat(HZ16x16[addr]);
			addr += 1;
		}
	}
}

/*显示单个字符的阳码，目前只能显示0~9
x:横坐标 0-127
y:纵坐标 0-7
data 字符形式的数字
*/
void OLED_Show_inverse(unsigned char x, unsigned char y,char data)
{
	unsigned char wm=0;
	OLED_SetPos(x , y);
	int addr = 0;
	addr = (data-48)*16;
	for(wm = 0;wm < 8;wm++)
	{
		WriteDat(F8X16_inverse[addr]);
		addr += 1;
	}
	OLED_SetPos(x,y+1);
	for(wm = 0;wm < 8;wm++)
	{
		WriteDat(F8X16_inverse[addr]);
		addr += 1;
	}
}

/*显示一个句子
参数
//x范围 0 ~　127  y 的范围 0~7
X，Y起始坐标
Disp_arr：句子的序列数组，在bsp_i2c_oled.h文件
length：句子的长度
clr_or_not:是否清屏，1清屏 0不清
默认显示16*16,只能显示一行*/
void Disp_sentence(uint8_t X,uint8_t Y,char *str,uint8_t clr_or_not)
{
	uint8_t X_now,Y_now;
	char * ptr;
	ptr = str;
	X_now = X;
	Y_now = Y;
	if (clr_or_not) OLED_CLS();
	// 字符串尾部
	while ((*ptr) != 0x00){
		// 显示的不是汉字
		if ((uint8_t)(*ptr) < 0x80){
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),1);
			X_now += 8;
			ptr+=1;
		}
		// 显示的是汉字
		else {
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),0);
			X_now += 16;
			ptr += 2;
		}
	}
}
void Disp_sentence_singleline(uint8_t X,uint8_t Y,char *str,uint8_t clr_or_not)
{
	uint8_t X_now,Y_now;
	char * ptr;
	ptr = str;
	X_now = X;
	Y_now = Y;
	if (clr_or_not){
		OLED_SetPos(0,Y_now);
		for (u8 i=0; i<128; i++){
			WriteDat(0x00);
		}
		OLED_SetPos(0,Y_now+1);
		for (u8 i=0; i<128; i++){
			WriteDat(0x00);
		}
	}
	// 字符串尾部
	while ((*ptr) != 0x00){
		// 显示的不是汉字
		if ((uint8_t)(*ptr) < 0x80){
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),1);
			X_now += 8;
			ptr+=1;
		}
		// 显示的是汉字
		else {
			OLED_Show(X_now,Y_now,*ptr,*(ptr+1),0);
			X_now += 16;
			ptr += 2;
		}
	}
}



static uint8_t code_111(uint8_t in)
{
	uint8_t code = 0x00;
	while(in>0)
	{
		code = (code<<1)|0x01;
		in--;
	}
	return code;
}
static uint8_t reverse(uint8_t in)
{
	int i=7;
	uint8_t code=0x00;
	while(i>0)
	{
		code  = (code|((in<<(7-i))&0x80))>>1;
		i--;
	}
	code  = (code|((in<<7)&0x80));
	return code;
}
//x 起始横坐标 0~127
//y 起始纵坐标 0~63
//length  长度；
//hv 0表示横线， 1表示纵线
void Disp_line(uint8_t xin,uint8_t yin,uint8_t length,uint8_t hv){
	uint8_t data = 0;
	int i=0;
	int j=0;
	uint8_t head,mid,tail;
	if (hv){
		if (length-1+yin > 63)
			length = 63+1-yin;
	}
	else {
		if (length-1+xin > 127)
			length = 127+1-xin;
	}
	if (hv){
		head = (yin/8+1)*8-yin;
		mid = (length - head)/8;
		tail = length - head - mid*8;
		OLED_SetPos(xin,yin/8);
		data = reverse(code_111(head));
		WriteDat(data);
		for (i=0;i<mid;i++){
			OLED_SetPos(xin,yin/8+1+i);
			WriteDat(0xff);
		}
		if (tail>0){
			OLED_SetPos(xin,yin/8+1+mid);
			data = code_111(tail);
			WriteDat(data);
		}
	}
	else{
		mid = length/8;
	  	tail = length - mid*8;
		OLED_SetPos(xin,yin/8);
		data = (yin/8+1)*8-yin;
		data = 0x80>>(data-1);

		OLED_SetPos(xin,yin/8);
		for (j=0;j<mid;j++)
			for(i=0;i<8;i++)
				WriteDat(data);
		OLED_SetPos(xin+mid*8,yin/8);
		for (i=0;i<8;i++){
			if(i>=tail)
				data = 0x00;
			WriteDat(data);
		}
	}
}

//name:		OLED显示电池电量函数
//func:		在右上角显示电池状态图标
//param:	power:	电池电量[0~4],0为缺电,4为满电
//return:	NONE
//brief:	NONE
void OLED_Show_Power(uint8_t power){
	OLED_SetPos(0,0);
	for (u8 i=0;i<111; i++){
		WriteDat(0x00);
	}
	OLED_SetPos(111,0);
	for(u8 i=0; i<17; i++){
		WriteDat(Logo_Power[power][i]);
	}
}

// 【已关门】界面显示，全屏写入，使用之前不用清屏
void show_clock_open_big(void){
	uint16_t temp;
	// 显示图标
	for (u8 hang=0; hang<8; hang++){
		OLED_SetPos(0,hang);
		temp = hang*128;
		for (u8 lie=0; lie<128; lie++){
			if (hang<6){
				WriteDat( IMG_OPEN_CLOCK[temp+lie] );
			}
			else{
				WriteDat( 0x00 );
			}
		}
	}
}

// 【已开门】界面显示，全屏写入，使用之前不用清屏
void show_clock_close_big(void){
	uint16_t temp;
	// 显示图标
	for (u8 hang=0; hang<8; hang++){
		OLED_SetPos(0,hang);
		temp = hang*128;
		for (u8 lie=0; lie<128; lie++){
			if (hang<6){
				WriteDat( IMG_CLOSE_CLOCK[temp+lie] );
			}
			else{
				WriteDat( 0x00 );
			}
		}
	}
}

// 在指定位置显示2448大小的数字。
// xin:	0~103
// yin: 0~2
int show_time_big(uint8_t xin,uint8_t yin,uint8_t numin){
	uint16_t temp,start;

	// 超出范围，不执行任务
	if (numin>9){
		return 0;
	}
	start = numin*144;
	for (u8 i=0; i<6; i++){
		OLED_SetPos(xin,yin+i);
		temp = i*24;
		for (u8 lie=0; lie<24; lie++){
			WriteDat( IMG_NUM_2448[start+temp+lie] );
		}
	}
	return 1;
}

// 显示时间界面上的俩个小点
void show_time_pot(void){
	uint16_t temp;
	for (u8 hang=0; hang<6; hang++){
		OLED_SetPos(52, hang+1);
		temp = hang*24;
		for (u8 lie=0; lie<24; lie++){
			WriteDat( IMG_POT[temp+lie] );
		}
	}
}

// 不显示时间界面上的俩个小点
void close_time_pot(void){
	for (u8 hang=0; hang<6; hang++){
		OLED_SetPos(52, hang+1);
		for (u8 lie=0; lie<24; lie++){
			WriteDat( 0x00 );
		}
	}
}

// 显示扳手图标 16*16
void show_hammer(uint8_t xin,uint8_t yin){
	uint16_t temp;
	for (u8 hang=0; hang<2; hang++){
		OLED_SetPos(xin, yin+hang);
		temp = hang*16;
		for (u8 lie=0; lie<16; lie++){
			WriteDat( IMG_HAMMER[temp+lie] );
		}
	}
}
