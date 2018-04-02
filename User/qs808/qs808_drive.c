#include  "./qs808/qs808_drive.h"


extern unsigned char HZ16x16[];
extern unsigned char F8X16[];
//注意：打开了Duplication Check ，当注册已有指纹时，会返回错误码ERR_DUPLICATION_ID，从而不能注册
//设计指纹库大小为200（1~200），所以在传递指纹库相关参数时，不得超过200

QS808_Rec_Buf_type QS808_Rec_Buf ={
	.Trans_state = reset,
	.Rec_state = idle,
	.Rec_point = 0,
	.Rec_Buf = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
extern uint32_t ms10_cnt;

//接收复位，接收到回包后，发送flag复位，接收状态复位，缓冲区指针复位
void QS808_Rec_Buf_refresh(void)
{
	QS808_Rec_Buf.Rec_state = idle;
	QS808_Rec_Buf.Rec_point = 0x00;
	QS808_Rec_Buf.Trans_state = reset;
}
// 解析qs808数据包
// 调用此函数后，定时3ms，如果没有数据，则直接返回空
// 返回0表示收包正确 返回0xff 表示收包错误
uint8_t QS808_Unpack(QS808_Pack_struct * qs808_rec_bag) {
	int i=0;
	uint16_t check=0;
	uint16_t check2=0;
	int timeout_cnt=0x500000;//大概1.5s
	while(QS808_Rec_Buf.Rec_state != busy)//等待qs808开始发送
	{
		timeout_cnt--;
		if(timeout_cnt<=0)
		{
			QS808_Rec_Buf_refresh();
			// Disp_sentence(0,0,"error6",1);
			// while(1){}
			return 0xff;
		}
	}
	delay_ms(3);//等待3ms  26*8/115200 = 1.8ms可以发送完成一帧
	QS808_Rec_Buf_refresh();
	for(i=0;i<24;i++){
		check += QS808_Rec_Buf.Rec_Buf[i];
	}
	// 校验和
	check2 = (QS808_Rec_Buf.Rec_Buf[25]<<8)|QS808_Rec_Buf.Rec_Buf[24];
	if(check != check2)
		return 0xff;
	qs808_rec_bag->HEAD = (QS808_Rec_Buf.Rec_Buf[1]<<8)|QS808_Rec_Buf.Rec_Buf[0];
	qs808_rec_bag->SID1 = (QS808_Rec_Buf.Rec_Buf[2]);
	qs808_rec_bag->DID1 = QS808_Rec_Buf.Rec_Buf[3];
	qs808_rec_bag->RCM = (QS808_Rec_Buf.Rec_Buf[5]<<8)|QS808_Rec_Buf.Rec_Buf[4];
	qs808_rec_bag->LEN = (QS808_Rec_Buf.Rec_Buf[7]<<8)|QS808_Rec_Buf.Rec_Buf[6];
	qs808_rec_bag->RET = (QS808_Rec_Buf.Rec_Buf[9]<<8)|QS808_Rec_Buf.Rec_Buf[8];
	for(i=0;i<14;i++)
		qs808_rec_bag->DATA[i] = QS808_Rec_Buf.Rec_Buf[i+10];
	qs808_rec_bag->CKS = check;
	return 0x00;
}

uint16_t QS808_send_byte(uint8_t byte)
{
	return Usart_SendByte(QS808_USART,byte);
}

uint16_t QS808_send_half_word(uint16_t half_word)
{
	if(Usart_SendByte(QS808_USART,half_word>>8))
		return 0xFFFF;
	if(Usart_SendByte(QS808_USART,half_word & 0x00ff))
		return 0xFFFF;
	return 0x0000;
}

uint8_t QS808_RecByte(void)
{
	 return Usart_RecvByte(QS808_USART);
}

void QS808_Reset(void)
{
	int i;
	QS808_NRST_High();
	for(i=0;i<1000;i++){}
	QS808_NRST_Low();
	for(i=0;i<1000;i++){}
	QS808_NRST_High();
	delay_ms(100);
}

void QS808_Init(void)
{
	QS808_UART_Init();
	//复位引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	QS808_NRST_GPIO_CLK, ENABLE );
	GPIO_InitStructure.GPIO_Pin = QS808_NRST_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(QS808_NRST_GPIO_PORT, &GPIO_InitStructure);
	//先复位QS808再写其他程序，这样给QS808启动反应时间
	//中断引脚
	GPIO_InitStructure.GPIO_Pin = QS808_INT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD ;
	GPIO_Init(QS808_INT_GPIO_PORT, &GPIO_InitStructure);

	//中断初始化
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
 	GPIO_EXTILineConfig(QS808_INT_PIN_SOURCE_PORT,QS808_INT_PIN_SOURCE_PIN);
	EXTI_InitStructure.EXTI_Line=QS808_INT_EXT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = QS808_INT_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	QS808_Reset();


}

uint8_t QS808_TEST_CONNECTION(void)
{
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_TEST_CONNECTION);//CMD
	QS808_send_half_word(0x0000);//LEN
	for (i=0;i<16;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x0001);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RET == ERR_SUCCESS)
		return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;
}
uint16_t QS808_CMD_GET_ENROLL_COUNT(void)
{
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_GET_ENROLL_COUNT);//CMD
	QS808_send_half_word(0x0400);//LEN
	QS808_send_half_word(0x0100);//start address
	QS808_send_half_word(0xF401);//end address
	for (i=0;i<12;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x4102);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
			return QS808_DATA_ERR;
	else
			return ((qs808_rec_bag.DATA[1]<<8)|(qs808_rec_bag.DATA[0]));

}

// 睡眠状态下会调用这个函数
uint8_t QS808_STANDBY(void)
{
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_STANDBY);//CMD
	QS808_send_half_word(0x0000);//LEN
	for (i=0;i<16;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x0B01);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
		return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;
}

uint8_t QS808_GET_IMAGE(void)
{
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_GET_IMAGE);//CMD
	QS808_send_half_word(0x0000);//LEN
	for (i=0;i<16;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x1f01);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RET == ERR_FP_NOT_DETECTED)
		return ERR_FP_NOT_DETECTED;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
		return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;
}

uint8_t QS808_CMD_GENERATE(uint16_t ram_id)
{
	QS808_Rec_Buf_refresh();
	//ram_id为16位，注意需要反着写，例如要将特征存入ram2，则ram_id为0x0200
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_GENERATE);//CMD
	QS808_send_half_word(0x0200);//LEN
	QS808_send_half_word(ram_id);//data
	for (i=0;i<14;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x6101+ram_id);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_BUFFER_ID)
		return ERR_INVALID_BUFFER_ID;
	else if (qs808_rec_bag.RET == ERR_BAD_QUALITY)
		return ERR_BAD_QUALITY;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
		return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;
}

uint8_t QS808_CMD_MERGE(void)
{
	QS808_Rec_Buf_refresh();
	//模板合成可调合成个数和目标ram，这里固定为3个指纹合成一个模板，存到ram0
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_MERGE);//CMD
	QS808_send_half_word(0x0300);//LEN
	QS808_send_half_word(0x0000);//合成到ram0里面
	QS808_send_byte(0x03);//3个指纹合成一个模板
	for (i=0;i<13;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x6601);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_BUFFER_ID)
		return ERR_INVALID_BUFFER_ID;
	else if (qs808_rec_bag.RET == ERR_GEN_COUNT)
		return ERR_GEN_COUNT;
	else if (qs808_rec_bag.RET == ERR_MERGE_FAIL)
		return ERR_MERGE_FAIL;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
		return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;

}


uint8_t QS808_CMD_FINGER_DETECT(void)
{
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_FINGER_DETECT);//CMD
	QS808_send_half_word(0x0000);//LEN
	for (i=0;i<16;i++)//data
	{
		QS808_send_byte(0x00);
	}

	QS808_send_half_word(0x2001);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.DATA[0]==0)
		return ERR_NO_FINGER_DETECT;
	else if(qs808_rec_bag.DATA[0]==1)
		return ERR_FINGER_DETECT;
	return QS808_UNKNOW_ERR;

}

uint8_t QS808_CMD_STORE_CHAR(uint16_t templete_num)
{
	//输入参数为模板的编号，不需要输入ram号，直接用ram0
	//模板编号高低位需要互反，比如存到第0x0011个编号，那么应传入参数0x1100
	//templete_num不得大于200
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_STORE_CHAR);//CMD
	QS808_send_half_word(0x0400);//LEN
	QS808_send_half_word(templete_num);//templete_num
	QS808_send_half_word(0x0000);//ramid

	for (i=0;i<12;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x4301+templete_num);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_TMPL_NO)
		return ERR_INVALID_TMPL_NO;
	else if (qs808_rec_bag.RET == ERR_INVALID_BUFFER_ID)
		return ERR_INVALID_BUFFER_ID;
	else if (qs808_rec_bag.RET == ERR_DUPLICATION_ID)
		return ERR_DUPLICATION_ID;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
		return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;
}

uint8_t QS808_CMD_GET_EMPTY_ID(uint8_t * ID)
{
	QS808_Rec_Buf_refresh();
	int i;
	*ID = 0x00;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_GET_EMPTY_ID);//CMD
	QS808_send_half_word(0x0400);//LEN
	QS808_send_half_word(0x0100);//起始地址
	QS808_send_half_word(0xc800);//结束地址
	for (i=0;i<12;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x1102);//sum check
	QS808_Rec_Buf.Trans_state = set;


	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_PARAM)
		return ERR_INVALID_PARAM;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
	{
		*ID = qs808_rec_bag.DATA[0];
		return ERR_SUCCESS;
	}
	return QS808_UNKNOW_ERR;
}

uint8_t	QS808_CMD_DEL_ALL(void)
 {
	//清空1~200的指纹
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_DEL_CHAR);//CMD
	QS808_send_half_word(0x0400);//LEN
	QS808_send_half_word(0x0100);//起始地址
	QS808_send_half_word(0xc800);//结束地址
	for (i=0;i<12;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x1002);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_PARAM)
		return ERR_INVALID_PARAM;
	else if (qs808_rec_bag.RET == ERR_TMPL_EMPTY)
		return ERR_TMPL_EMPTY;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
		return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;

}

uint8_t	QS808_CMD_SEARCH(uint8_t * index)
{
	//实验发现，这个模块在指纹被清空后，并不会返回ERR_ALL_TMPL_EMPTY这个错误
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_SEARCH);//CMD
	QS808_send_half_word(0x0600);//LEN
 	QS808_send_half_word(0x0000);//指纹模板在ram0里
	QS808_send_half_word(0x0100);//起始地址 1
	QS808_send_half_word(0xc800);//结束地址 200
	for (i=0;i<10;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x3102);//sum check
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_BUFFER_ID)
		return ERR_INVALID_BUFFER_ID;
	else if (qs808_rec_bag.RET == ERR_ALL_TMPL_EMPTY)
		return ERR_ALL_TMPL_EMPTY;
	else if (qs808_rec_bag.RET == ERR_IDENTIFY)
		return ERR_IDENTIFY;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
	{
		*index = qs808_rec_bag.DATA[0];
		return ERR_SUCCESS;
	}
	return QS808_UNKNOW_ERR;
}
extern uint32_t ms10_cnt;

uint8_t QS808_Wait(uint32_t time) {
	// 等待指纹输入函数，超时返回QS808_WAIT_TIMEOUT，否则返回0x00
	// time是以毫秒为单位的超时时间
	// 测试发现，没有手指输入时，指纹模块有500ms的等待，所以不需要再认为添加等待时间
	uint8_t temp;
	uint32_t cnt = time/180;//扫描圈数
	while (cnt>0) {
		temp = QS808_GET_IMAGE();
		if (temp == ERR_FP_NOT_DETECTED) {
			cnt--;
			continue;
		}
		else if (temp == ERR_SUCCESS) return ERR_SUCCESS;
		else return QS808_DATA_ERR;
	}
	return QS808_WAIT_TIMEOUT;
}

// 返回值：
// 1.QS808_WAIT_TIMEOUT
// 2.ERR_BAD_QUALITY
// 3.ERR_MERGE_FAIL
// 4.ERR_DUPLICATION_ID
// 5.ERR_SUCCESS
// 参数是u8指针,用于输出指纹的ID,ID从1开始
uint8_t QS808_Login(uint8_t* ID) {
	uint8_t temp;
	int i=0;

	// 采集3次指纹信息
	for (i=0;i<3;i++){
		// if (i==0)	Disp_sentence(20,2,"请按手指",1);
		// else		Disp_sentence(15,2,"请重复按手指",1);
		// 采集指纹
		temp = QS808_Wait(10000);
		if(temp == QS808_WAIT_TIMEOUT){
			return QS808_WAIT_TIMEOUT;
		}
		// 生成特征
		temp = QS808_CMD_GENERATE((uint16_t)(i<<8));
		if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_BUFFER_ID))
			return QS808_DATA_ERR;
		if (temp == ERR_BAD_QUALITY)
			return ERR_BAD_QUALITY;
		// Disp_sentence(15,2,"指纹采集成功",1);
		delay_ms(1000);
	}

	// 合并特征
	temp = QS808_CMD_MERGE();

	// 数据不正常
	if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_BUFFER_ID) || (temp == ERR_GEN_COUNT))
		return QS808_DATA_ERR;

	// 合并特征失败
	if (temp == ERR_MERGE_FAIL)
		return ERR_MERGE_FAIL;

	// 获取可注册的ID
	temp = QS808_CMD_GET_EMPTY_ID(ID);

	// 数据不正常
	if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_PARAM))
		return QS808_DATA_ERR;

	// 入库
	temp = QS808_CMD_STORE_CHAR((uint16_t)(*ID<<8));

	// 数据不正常
	if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_TMPL_NO) || (temp == ERR_INVALID_BUFFER_ID))
		return QS808_DATA_ERR;

	// 指纹已存在
	if (temp == ERR_DUPLICATION_ID)	return ERR_DUPLICATION_ID;

	// 能够跑到这里就相当于成功了
	return ERR_SUCCESS;
}
uint8_t QS808_SEARCH(uint8_t * index)
{
	uint8_t temp;
	temp = QS808_Wait(1000);//采集指纹

	if(temp == QS808_WAIT_TIMEOUT)
	{
		//printf("等待超时\n");
		return QS808_WAIT_TIMEOUT;
	}
	temp = QS808_CMD_GENERATE((uint16_t)(0x0000));//生成特征到ram0
	if((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_BUFFER_ID))
		return QS808_DATA_ERR;
	else if (temp == ERR_BAD_QUALITY)
		return ERR_BAD_QUALITY;

	temp = QS808_CMD_SEARCH(index);//搜索指纹

	if((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_BUFFER_ID))
			return QS808_DATA_ERR;
	else if (temp == ERR_ALL_TMPL_EMPTY)
		return ERR_ALL_TMPL_EMPTY;
	else if(temp == ERR_IDENTIFY)
		return ERR_IDENTIFY;
	else
		return ERR_SUCCESS;
}


uint8_t	QS808_CMD_DEL_NUM(void)
{
	QS808_Rec_Buf_refresh();
	//清空固定编号的指纹
	uint8_t index;
	uint8_t temp;
	uint16_t sum;
	int i;
	temp = QS808_SEARCH(&index);//先得到指纹编号
	if(temp == ERR_SUCCESS)
	{
	sum = 0x0147+index+index;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_DEL_CHAR);//CMD
	QS808_send_half_word(0x0400);//LEN
	QS808_send_half_word((uint16_t) (index<<8));//起始地址
	QS808_send_half_word((uint16_t) (index<<8));//结束地址
	for (i=0;i<12;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_byte(sum);//sum check
	QS808_send_byte(sum>>8);
	QS808_Rec_Buf.Trans_state = set;

	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_PARAM)
	{
		return ERR_INVALID_PARAM;
	}
	else if (qs808_rec_bag.RET == ERR_TMPL_EMPTY)
	{
//		printf("删除失败\n");
		return ERR_TMPL_EMPTY;
	}
	else if (qs808_rec_bag.RET == ERR_SUCCESS)
	{
//		printf("删除成功\n");
		return ERR_SUCCESS;
	}
		return QS808_UNKNOW_ERR;
	}
	else
	{
//		printf("删除失败\n");
		return ERR_DEL_FAIL;
	}



}

// 与QS808_CMD_DEL_NUM的区别在于 QS808_CMD_DEL_NUM 是按手指 然后删除。QS808_CMD_DEL_NUM2是根据指纹ID删除指纹
uint8_t	QS808_CMD_DEL_NUM2(uint8_t index) {
	// 清空固定编号的指纹
	uint16_t sum;
	int i;
	QS808_Rec_Buf_refresh();
	sum = 0x0147+index+index;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_DEL_CHAR);//CMD
	QS808_send_half_word(0x0400);//LEN
	QS808_send_half_word((uint16_t) (index<<8));//起始地址
	QS808_send_half_word((uint16_t) (index<<8));//结束地址
	for (i=0;i<12;i++) {
		QS808_send_byte(0x00);
	}

	// sum checks
	QS808_send_byte(sum);
	QS808_send_byte(sum>>8);
	QS808_Rec_Buf.Trans_state = set;
	QS808_Pack_struct qs808_rec_bag;
	
	// 通信失败
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)			return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)					return QS808_DATA_ERR;
	else if (qs808_rec_bag.RET == ERR_INVALID_PARAM)	return ERR_INVALID_PARAM;
	else if (qs808_rec_bag.RET == ERR_TMPL_EMPTY)		return ERR_TMPL_EMPTY;
	else if (qs808_rec_bag.RET == ERR_SUCCESS)			return ERR_SUCCESS;
	return QS808_UNKNOW_ERR;
}

// 发送寻找手指指令,只发送，不接收,主界面用
void QS808_Detect_Finger(void)
{
	QS808_Rec_Buf_refresh();
	int i;
	QS808_send_half_word(QS808_HEAD);//包头
	QS808_send_byte(SID);
	QS808_send_byte(DID);
	QS808_send_half_word(CMD_FINGER_DETECT);//CMD
	QS808_send_half_word(0x0000);//LEN
	for (i=0;i<16;i++)//data
	{
		QS808_send_byte(0x00);
	}
	QS808_send_half_word(0x2001);//sum check
	QS808_Rec_Buf.Trans_state = set;

}

uint8_t QS808_Detect_Finger_Unpack(void)//解析寻找手指结果，主界面用
{
	QS808_Pack_struct qs808_rec_bag;
	if(QS808_Unpack(&qs808_rec_bag) == 0xff)//通信失败
		return QS808_DATA_ERR;
	else if(qs808_rec_bag.RCM == 0xff)
		return QS808_DATA_ERR;
	else if (qs808_rec_bag.DATA[0]==0)
		return ERR_NO_FINGER_DETECT;
	else if(qs808_rec_bag.DATA[0]==1)
		return ERR_FINGER_DETECT;
	return QS808_UNKNOW_ERR;

}
