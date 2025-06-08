#include "sys.h"

void Scence_Show(void)
{
	char k1=0;
	char Show_array1[]={"Balance_Car"};
	
	for(k1=0;k1<11;k1++)
	{
		if(k1>0 && k1<4)
		{
			delay_ms(500);
			OLED_ShowChar(2,3+k1,Show_array1[k1]);
		}
		else
		{
			delay_ms(200);
			OLED_ShowChar(2,3+k1,Show_array1[k1]);
		}		
	}
	delay_ms(500);
	OLED_Clear();
}

void SYS_Init(void)
{	
	NVIC_Configuration();
	delay_init();	    							 //延时函数初始化	
	Motor_Init();	
	LED_Init();
	OLED_Init();
	OLED_Clear();	
//	delay_ms(2000);
//	Scence_Show();
	
	Encoder_Init_TIM2();					 //=====编码器2初始化
	Encoder_Init_TIM3();					 //=====编码器3初始化
	uart_init(128000);	 	                         //串口初始化为115200	
	uart2_init(115200);
	TIM4_Int_Init(9999,7199);
	MPU_Init();	                                     //初始化MPU6050
	MPU6050_EXTI_Init();
	while(mpu_dmp_init())                            //初始化mpu_dmp库
 	{
		printf("Initialization failed！\r\n");		//串口初始化失败上报
	}
	printf("Initialization succeed！\r\n");			//串口初始化成功上报

}

void NVIC_Configuration(void)
{
		NVIC_InitTypeDef  NVIC_InitStructure;	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级

		//////////////////外部中断5优先级配置也就是MPU6050 INT引脚的配置
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;				//使能外部中断通道
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;	//抢占优先级0， 
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
		NVIC_Init(&NVIC_InitStructure); 
	
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;  //先占优先级1级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;  //从优先级3级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
		NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;//抢占优先级2
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;		//子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
}


