#include "./RFCard/rfcard.h"
#include "./RC522/rc522_function.h"
#include "./data/data_def.h"

// 添加射频卡
// result:  添加成功或者失败
u16 Add_RFCard(void) {
    u32 RFCARD_ID;
    u8 rfcard;
    
    // 开始检测射频卡
    rfcard = IC_test(&RFCARD_ID);
    
    // 如果检测到了射频卡，那么就开始将卡号存入 Flash
    if (rfcard == RFCARD_DETECED){
        return ERROR_CODE_SUCCESS;
    }
    
    return ERROR_CODE_TIMEOUT;
}
