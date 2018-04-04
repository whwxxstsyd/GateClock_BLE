#ifndef __FINGER_H
#define __FINGER_H
#include "stm32f10x.h"

u16 Add_Finger(u16* user_id);
u16 Delete_Finger(u16 user_number);
u16 Confirm_Finger(void);


#endif
