#ifndef __RFCARD_H
#define __RFCARD_H
#include "stm32f10x.h"



u16 Add_RFCard(u16* user_number);
u16  Delete_RFCard(u16 user_number);
u16 Confirm_RFCard(u32 RFCARD_ID);
u8 RFCard_test(uint32_t* RFCARD_ID);

#endif
