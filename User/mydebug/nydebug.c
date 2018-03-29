#include "my_board.h"
#include "./mydebug/mydebug.h"
extern uint8_t debug_data_read_flag;

void flashdata2usart(void)
{
	int i;
	int j;
	MY_USER my_user1;
	uint32_t addr;
	uint16_t u16temp;
	debug_data_read_flag=0;
	//指纹个数
	u16temp = QS808_CMD_GET_ENROLL_COUNT();
	printf("There are %d fingers in the QS808\n",u16temp);
	printf("----------------------------------\n");	
	//先读出用户个数
	STMFLASH_Read(INIT_ADDR,(uint16_t*)&u16temp,1); 
	printf("INIT_WORD is %x\n",u16temp);
	
	STMFLASH_Read(alluser_amount_addr,(uint16_t*)&u16temp,1); 
	printf("alluser_amount is %d\n",u16temp);
	
	STMFLASH_Read(admin_amount_addr,(uint16_t*)&u16temp,1); 
	printf("admin_amount is %d\n",u16temp);
	
	STMFLASH_Read(family_amount_addr,(uint16_t*)&u16temp,1); 
	printf("family_amount is %d\n",u16temp);
	
	STMFLASH_Read(babysitter_amount_addr,(uint16_t*)&u16temp,1); 
	printf("babysitter_amount is %d\n",u16temp);
	printf("----------------------------------\n");	
	//保姆开门时间
	UNLOCK_TIME utime_debug;
	STMFLASH_Read(UNLOCK_TIME_addr,(uint16_t*)&utime_debug,UNLOCK_TIME_length/2);
	printf("time start:\n");
	printf("minute:%d,second:%d\n",utime_debug.hour,utime_debug.minute);
	printf("time stop:\n");
	printf("minute:%d,second:%d\n",utime_debug.hour2,utime_debug.minute2);
	printf("----------------------------------\n");		

	for (i=0;i<MY_USER_MAX_NUM;i++)
	{
		addr = (uint32_t)MY_USER_addr_base + i*MY_USER_length;
		STMFLASH_Read(addr,(uint16_t*)&my_user1,MY_USER_length/2); //注意第三个参数是半字数，这很关键，这里写错，导致my_user1越界,所以不论my_user1用局部变量还是malloc都是不对的。
		printf("address:%x No.%d\n",addr,i);
		printf("flag:%x\n",my_user1.flag);
		printf("type:%d\n",my_user1.my_user_type);
		printf("number:%d\n",my_user1.number);
		printf("finger_number:%d\n",my_user1.finger_number);
		printf("rfcard_id:%x\n",my_user1.rfcard_id);
		printf("password_length:%d\n",my_user1.password_length);
		printf("password is:\n");
		for (j=0;j<102;j++)
		{
			printf("%d,",my_user1.password[j]);
		}
		printf("\n");
		printf("time start:\n");
		printf("year:%d,month:%d,date:%d,hour:%d,minute:%d,second:%d\n",my_user1.time_start.year,my_user1.time_start.month,my_user1.time_start.date,my_user1.time_start.hour,my_user1.time_start.minute,my_user1.time_start.second);
		printf("time stop:\n");
		printf("year:%d,month:%d,date:%d,hour:%d,minute:%d,second:%d\n",my_user1.time_stop.year,my_user1.time_stop.month,my_user1.time_stop.date,my_user1.time_stop.hour,my_user1.time_stop.minute,my_user1.time_stop.second);		
		printf("----------------------------------\n");		
	}

}
