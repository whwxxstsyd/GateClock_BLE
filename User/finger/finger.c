#include "./finger/finger.h"
#include "./qs808/qs808_drive.h"
#include "./data/data_def.h"                    // 为了使用用户结构体
#include "./STMFLASH/stmflash.h"               // 为了向 flash 中写入射频卡编号
#include "./UT588C/ut588c.h"					// 为了使用喇叭
#include "./usart/debug_usart.h"







// 添加指纹
// UserID:	    存储分配好的十进制用户number
// return:  	添加成功或者失败
u16 Add_Finger(u16* user_id) {
    u8 temp;
    u8 qs808_finger_id;
    u16 temp_user_number;
    u32 addr_now;
    MY_USER user;

    // 应手机端的要求，刚开始给他先发一个成功的回包
    Usart_SendFinger_ADD_Success(USART1, 0, 0);

    /******************************* 采集3次指纹信息 *******************************/
    for (u8 i=0; i<3; i++) {
        // 采集指纹图像，如果超时就向手机发送数据包头【ERROR_CODE_TIMEOUT】，并且return退出
        if (QS808_Wait(10000) == QS808_WAIT_TIMEOUT) {
            Usart_SendFinger_ADD_Error(USART1, i+1, ERROR_CODE_TIMEOUT);
            return ERROR_CODE_TIMEOUT;
        }

        // 生成指纹特征
        temp = QS808_CMD_GENERATE((uint16_t)(i<<8));

        // 如果生成指纹特征出错，就向手机发送数据包头【ERROR_CODE_TIMEOUT】，并且return退出
        if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_BUFFER_ID)) {
            Usart_SendFinger_ADD_Error(USART1, i+1, ERROR_CODE_TIMEOUT);
            return ERROR_CODE_TIMEOUT;
        }

        // 如果指纹质量差，就向手机发送数据包头【ERROR_CODE_FINGERBAD】，并且return退出
        else if (temp == ERR_BAD_QUALITY) {
            Usart_SendFinger_ADD_Error(USART1, i+1, ERROR_CODE_FINGERBAD);
            return ERROR_CODE_FINGERBAD;
        }

        // 如果能跑到这里就说明采集成功，就向手机端发送数据包头【ERROR_CODE_SUCCESS】
        else {
            // 第三次的包头先不发，因为第三次的包头需要根据最终的合成以及入库的结果来,这个时候的user_id随便给就行
            if(i<2)    Usart_SendFinger_ADD_Success(USART1, i+1, 0);
        }

        // 暂时还不知道这个 delay 的用处是什么
        delay_ms(1000);
    }


    /********************************** 合并特征 **********************************/
	temp = QS808_CMD_MERGE();
    // 如果合并特征数据不正常，就向手机发送数据包头【ERROR_CODE_FINGERBAD】，并且return退出
	if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_BUFFER_ID) || (temp == ERR_GEN_COUNT) || (temp == ERR_MERGE_FAIL)) {
        Usart_SendFinger_ADD_Error(USART1, 3, ERROR_CODE_FINGERBAD);
		return ERROR_CODE_FINGERBAD;
    }


    /******************************* 获取可注册的ID *******************************/
	temp = QS808_CMD_GET_EMPTY_ID(&qs808_finger_id);
	// 如果获取可注册的ID出错，就向手机发送数据包头【ERROR_CODE_TIMEOUT】，并且return退出
	if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_PARAM)) {
        Usart_SendFinger_ADD_Error(USART1, 3, ERROR_CODE_TIMEOUT);
	    return ERROR_CODE_TIMEOUT;
    }


    /************************************ 入库 ************************************/
	temp = QS808_CMD_STORE_CHAR((uint16_t)(qs808_finger_id<<8));
	// 如果入库不正常，就向手机发送数据包头【ERROR_CODE_TIMEOUT】，并且return退出
	if ((temp == QS808_DATA_ERR) || (temp == QS808_UNKNOW_ERR) || (temp == ERR_INVALID_TMPL_NO) || (temp == ERR_INVALID_BUFFER_ID)) {
        Usart_SendFinger_ADD_Error(USART1, 3, ERROR_CODE_TIMEOUT);
		return ERROR_CODE_TIMEOUT;
    }
	// 如果指纹已存在，就向手机发送数据包头【ERROR_CODE_DUPLICATION】，并且return退出
	if (temp == ERR_DUPLICATION_ID)	{
        Usart_SendFinger_ADD_Error(USART1, 3, ERROR_CODE_DUPLICATION);
        return ERROR_CODE_DUPLICATION;
    }


    /********************************** 开始写入 **********************************/
	// 能够跑到这里就近似相当于成功了，就向flash中写入 ID 数据，flash里面存的就是 ID。
    // 但是如果发现 flash 已经存满了,没有空间了,那还是得报错
    for (u16 i=0; i<1000; i++) {
        addr_now = MY_USER_ADDR_START +i*MY_USER_LENGTH;
        STMFLASH_Read(addr_now, &temp_user_number, 1);

        // 如果已经扫描到了空白区域，那就直接写入数据，并且 return ERROR_CODE_SUCCESS
        if ( temp_user_number == 0xFFFF ) {
            user.m_USER_Number = i;
            user.m_USER_Type = MY_USER_FINGER;
            user.m_USER_Data = qs808_finger_id;
            STMFLASH_Write(addr_now, (u16*)&user, MY_USER_LENGTH/2);
            *user_id = i;
            return ERROR_CODE_SUCCESS;
        }
    }

    // 能跑到这里,说明 flash 已经没有空间再写入新的信息了,return ERROR_CODE_TIMEOUT
    Usart_SendFinger_ADD_Error(USART1, 3, ERROR_CODE_TIMEOUT);
	return ERROR_CODE_TIMEOUT;
}


// 删除指纹
// user_number:	需要删除的十进制用户编号
// return:  	添加成功或者失败
u16 Delete_Finger(u16 user_number) {
	u16 no_user = 0xFFFF;
    MY_USER user;
    u32 addr_now;

    addr_now = MY_USER_ADDR_START +user_number*MY_USER_LENGTH;
    STMFLASH_Read(addr_now, (u16*)&user, MY_USER_LENGTH/2);

	// 判断用户编号合法性,如果编号合法,删除用户,return ERROR_CODE_SUCCESS
	if (user_number<=999 && user_number==user.m_USER_Number) {
		// 将原有的用户编号置为0xFFFF,就相当于删除了弹片击中这个编号下的所有数据
		STMFLASH_Write(addr_now, &no_user, 1);

        // 然后还需要删除指纹头中的数据,避免之后出现指纹头中有用户数据,然后flash中没有用户数据的情况
        // 如果删除成功,那就 return ERROR_CODE_SUCCESS
        if (QS808_CMD_DEL_NUM2(user.m_USER_Data) == ERR_SUCCESS)
    		return ERROR_CODE_SUCCESS;
        else
            return ERROR_CODE_ERROR;
	}
	// 如果编号非法，return ERROR_CODE_ERROR
	else {
		return ERROR_CODE_ERROR;
	}
}
