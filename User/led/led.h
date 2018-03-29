#ifndef __LED_H
#define	__LED_H

//引脚定义
#define LED1_PORT                GPIOB
#define LED1_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED1_PIN                 GPIO_Pin_8
#define LED1_OFF() 								GPIO_SetBits(LED1_PORT,LED1_PIN);
#define LED1_ON() 								GPIO_ResetBits(LED1_PORT,LED1_PIN);	



#define LED2_PORT                GPIOB
#define LED2_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED2_PIN                 GPIO_Pin_9
#define LED2_OFF() 								GPIO_SetBits(LED2_PORT,LED2_PIN);
#define LED2_ON() 								GPIO_ResetBits(LED2_PORT,LED2_PIN);	


#define LED3_PORT                GPIOA
#define LED3_GPIO_CLK            RCC_APB2Periph_GPIOA
#define LED3_PIN                 GPIO_Pin_6
#define LED3_OFF() 								GPIO_SetBits(LED3_PORT,LED3_PIN);
#define LED3_ON() 								GPIO_ResetBits(LED3_PORT,LED3_PIN);	


#define LED4_PORT                GPIOB
#define LED4_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED4_PIN                 GPIO_Pin_5
#define LED4_OFF() 								GPIO_SetBits(LED4_PORT,LED4_PIN);
#define LED4_ON() 								GPIO_ResetBits(LED4_PORT,LED4_PIN);	


#define LED5_PORT                GPIOB
#define LED5_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED5_PIN                 GPIO_Pin_4
#define LED5_OFF() 								GPIO_SetBits(LED5_PORT,LED5_PIN);
#define LED5_ON() 								GPIO_ResetBits(LED5_PORT,LED5_PIN);	



#define LED6_PORT                GPIOA
#define LED6_GPIO_CLK            RCC_APB2Periph_GPIOA
#define LED6_PIN                 GPIO_Pin_7
#define LED6_OFF() 								GPIO_SetBits(LED6_PORT,LED6_PIN);
#define LED6_ON() 								GPIO_ResetBits(LED6_PORT,LED6_PIN);	



#define LED7_PORT                GPIOB
#define LED7_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED7_PIN                 GPIO_Pin_3
#define LED7_OFF() 								GPIO_SetBits(LED7_PORT,LED7_PIN);
#define LED7_ON() 								GPIO_ResetBits(LED7_PORT,LED7_PIN);	



#define LED8_PORT                GPIOA
#define LED8_GPIO_CLK            RCC_APB2Periph_GPIOA
#define LED8_PIN                 GPIO_Pin_8
#define LED8_OFF() 								GPIO_SetBits(LED8_PORT,LED8_PIN);
#define LED8_ON() 								GPIO_ResetBits(LED8_PORT,LED8_PIN);	



#define LED9_PORT                GPIOB
#define LED9_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED9_PIN                 GPIO_Pin_12
#define LED9_OFF() 								GPIO_SetBits(LED9_PORT,LED9_PIN);
#define LED9_ON() 								GPIO_ResetBits(LED9_PORT,LED9_PIN);	



#define LED10_PORT                GPIOB
#define LED10_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED10_PIN                 GPIO_Pin_15
#define LED10_OFF() 								GPIO_SetBits(LED10_PORT,LED10_PIN);
#define LED10_ON() 								GPIO_ResetBits(LED10_PORT,LED10_PIN);	



#define LED0_PORT                GPIOB
#define LED0_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED0_PIN                 GPIO_Pin_14
#define LED0_OFF() 								GPIO_SetBits(LED0_PORT,LED0_PIN);
#define LED0_ON() 								GPIO_ResetBits(LED0_PORT,LED0_PIN);	



#define LED11_PORT                GPIOB
#define LED11_GPIO_CLK            RCC_APB2Periph_GPIOB
#define LED11_PIN                 GPIO_Pin_13
#define LED11_OFF() 								GPIO_SetBits(LED11_PORT,LED11_PIN);
#define LED11_ON() 								GPIO_ResetBits(LED11_PORT,LED11_PIN);	



//宏



//函数原型
void led_init(void);
void led_on_all(void);
void led_off_all(void);
#endif

