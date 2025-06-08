#include "led.h"   
    

	
//LED IO≥ı ºªØ
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	    		
 GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 
 GPIO_SetBits(GPIOB,GPIO_Pin_0); 						
}
 
void led_on(void)
{
	LED=0;
}
void led_off(void)
{
	LED=1;
}

//PWMøÿ÷∆

