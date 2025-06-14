#include "Motor.h"

void Motor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	PWM_Init();
}


/**************************************************************************
CN2:R PWMA PA8 TIM1_CH1
CN3:L PWMB PA11 TIM1_CH4
**************************************************************************/
void Set_Pwm(int moto1,int moto2)
{
	if(moto1<0)		AIN1=0,			AIN2=1;
	else 	        AIN1=1,			AIN2=0;
	PWMA=myabs(moto1);
	
	if(moto2<0)		BIN1=0,			BIN2=1;
	else        	BIN1=1,			BIN2=0;
	PWMB=myabs(moto2);	
}

int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}

void Xianfu_Pwm(void)
{
	 //===PWM������7200 
    if(Moto1<-7000 ) Moto1=-7000 ;
	if(Moto1>7000 )  Moto1=7000 ;
	if(Moto2<-7000 ) Moto2=-7000 ;
	if(Moto2>7000 )  Moto2=7000 ;
}


	
