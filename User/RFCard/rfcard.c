#include "./RFCard/rfcard.h"
#include "./RC522/rc522_function.h"
#include "./data/data_def.h"                    // 为了使用用户结构体
#include "./STMFLASH/stmflash.h"                // 为了向 flash 中写入射频卡编号
#include "./UT588C/ut588c.h"


// 添加射频卡
// user_number:	存储分配好的十进制用户number
// return:  	添加成功或者失败
u16 Add_RFCard(u16* user_number) {
	u32 RFCARD_ID,addr_now;
	u16 temp_user_number;
	u8 temp;
	MY_USER user;

	// 开始检测射频卡
	temp = IC_test(&RFCARD_ID);

	// 如果检测到了射频卡
	if (temp == RFCARD_DETECED) {
		// 开始从首地址开始查找这张卡有没有被录入过
		for (u16 i=0; i<1000; i++) {
			addr_now = MY_USER_ADDR_START +i*MY_USER_LENGTH;
			STMFLASH_Read(addr_now, (u16*)&user, MY_USER_LENGTH/2);
			if (user.m_USER_Number!=0xFFFF &&user.m_USER_Type==MY_USER_RFCARD &&user.m_USER_Data==RFCARD_ID) {
				// 表示这张卡已经录入过了，直接 return ERROR_CODE_DUPLICATION
				return ERROR_CODE_DUPLICATION;
			} 
		}

		// 能够跑到这里就说明这张卡没有被录入过，那就直接找个空的 flash 写入就行了
		for (u16 i=0; i<1000; i++) {
			addr_now = MY_USER_ADDR_START +i*MY_USER_LENGTH;
			STMFLASH_Read(addr_now, &temp_user_number, 1);
			
			// 如果已经扫描到了空白区域，那就直接写入数据，并且 return ERROR_CODE_SUCCESS
			if ( temp_user_number == 0xFFFF ) {
				user.m_USER_Number = i;
				user.m_USER_Type = MY_USER_RFCARD;
				user.m_USER_Data = RFCARD_ID;
				STMFLASH_Write(addr_now, (u16*)&user, MY_USER_LENGTH/2);
				*user_number = i;
				return ERROR_CODE_SUCCESS;
			}
		    else {
				// 如果扫描到说是以前已经存过这张卡了，就直接 return ERROR_CODE_TIMEOUT，相当于录入失败 
				if ( user.m_USER_Type==MY_USER_RFCARD && user.m_USER_Data==RFCARD_ID) {
					return ERROR_CODE_TIMEOUT;
				}
			}
		}
		// 如果能跑到这里，那就说明flash已经占满了，就直接 return ERROR_CODE_TIMEOUT，相当于录入失败
		return ERROR_CODE_TIMEOUT;
	}

	// 没有检测到射频卡，就直接 return ERROR_CODE_TIMEOUT，相当于录入失败
	return ERROR_CODE_TIMEOUT;
}

// 删除射频卡
// user_number:	需要删除的十进制用户编号
// return:  	添加成功或者失败
u16 Delete_RFCard(u16 user_number) {
	u16 no_user = 0xFFFF;
	
	// 判断用户编号合法性， 如果编号合法，删除用户，return ERROR_CODE_SUCCESS
	if (user_number<=999) {
		// 将原有的用户编号置为 0xFFFF，就相当于删除了这个编号下的所有数据
		STMFLASH_Write(MY_USER_ADDR_START +user_number*MY_USER_LENGTH, &no_user, 1);
		return ERROR_CODE_SUCCESS;
	}
	// 如果编号非法，return ERROR_CODE_ERROR
	else {
		return ERROR_CODE_ERROR;
	}
}

// 射频卡验证，判断正在接触的这张射频卡有没有被事先存过
// RFCARD_ID:	正在接触的这张射频卡的物理ID
// reuturn:		解锁失败或成功
u16 Confirm_RFCard(u32 RFCARD_ID) {
	u32 addr_now;
	MY_USER user;
	
	// 开始从首地址开始查找这张卡有没有被录入过
	for (u16 i=0; i<1000; i++) {
		addr_now = MY_USER_ADDR_START +i*MY_USER_LENGTH;
		STMFLASH_Read(addr_now, (u16*)&user, MY_USER_LENGTH/2);
		if (user.m_USER_Number!=0xFFFF &&user.m_USER_Type==MY_USER_RFCARD &&user.m_USER_Data==RFCARD_ID) {
			// 表示这张卡已经录入过了，直接 return ERROR_CODE_SUCCESS
			return ERROR_CODE_SUCCESS;
		} 
	}

	// 能跑到这里，说明这张卡并没有被录过，就直接 return ERROR_CODE_ERROR
    return ERROR_CODE_ERROR;
}

// 检测当前是否有射频卡放置
// return : RFCARD_DETECED NO_RFCARD
u8 RFCard_test(uint32_t* RFCARD_ID) {
	u8 ucArray_ID [4];  // 先后存放IC卡的类型和UID(IC卡序列号)
	u8 ucStatusReturn;	// 返回状态
	*RFCARD_ID = 0x00000000;
	int counter;
	uint8_t flag = NO_RFCARD;
	
	counter = 0;
	while(1){
		ucStatusReturn = PcdRequest(PICC_REQALL, ucArray_ID);
		if (ucStatusReturn==MI_OK) {
			break;
		}
		counter++;
		if (counter>2) {
			break;
		}
	}

	if (ucStatusReturn == MI_OK) {
		// 防冲撞（当有多张卡进入读写器操作范围时，防冲突机制会从其中选择一张进行操作）
		if (PcdAnticoll(ucArray_ID) == MI_OK) {
			PcdAntennaOff();
			flag = RFCARD_DETECED;
			for (u8 i=0;i<4;i++){
				*RFCARD_ID = (*RFCARD_ID<<8)|ucArray_ID[i];
			}
		}
	}
	return flag;
}
