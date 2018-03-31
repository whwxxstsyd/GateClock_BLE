#include "./RFCard/rfcard.h"
#include "./RC522/rc522_function.h"
#include "./data/data_def.h"                    // 为了使用用户结构体
#include "./STMFLASH/stmflash.h"                // 为了向 flash 中写入射频卡编号

// 添加射频卡
// result:  添加成功或者失败
u16 Add_RFCard(u16* user_number) {
	u32 RFCARD_ID,addr_now;
	u16 buf_user_number,buf_user_type;
	u8 temp;
	MY_USER user;

	// 开始检测射频卡
	temp = IC_test(&RFCARD_ID);

	// 如果检测到了射频卡
	if (temp == RFCARD_DETECED) {
		// 开始从首地址开始查找这张卡有没有被录入过
		for (u16 i=0; i<1000; i++) {
			addr_now = MY_USER_ADDR_START +i*MY_USER_LENGTH;
			STMFLASH_Read(addr_now, (u16*)&user, 4);
			
			// 如果已经扫描到了空白区域，那就直接写入数据，并且 return ERROR_CODE_SUCCESS，相当于录入成功
			if ( user.m_USER_Number == 0xFFFF ) {
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
		// 如果能把整个1000循环跑完，那就说明flash已经占满了，就直接 return ERROR_CODE_TIMEOUT，相当于录入失败
		return ERROR_CODE_TIMEOUT;
	}

	// 没有检测到射频卡，就直接 return ERROR_CODE_TIMEOUT，相当于录入失败
	return ERROR_CODE_TIMEOUT;
}
