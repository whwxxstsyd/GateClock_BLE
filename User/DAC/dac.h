#ifndef __DAC_H
#define __DAC_H	 	    
#include "stm32f10x_dac.h"
#include "stm32f10x_rcc.h"		
#include "stm32f10x.h"						    

void Dac1_Init(void);//回环模式初始化		 	 
void Dac1_Set_Vol(u16 vol);
#endif

















