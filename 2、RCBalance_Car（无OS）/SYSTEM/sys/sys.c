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
	delay_init();	    							 //��ʱ������ʼ��	
	Motor_Init();	
	LED_Init();
	OLED_Init();
	OLED_Clear();	
//	delay_ms(2000);
//	Scence_Show();
	
	Encoder_Init_TIM2();					 //=====������2��ʼ��
	Encoder_Init_TIM3();					 //=====������3��ʼ��
	uart_init(128000);	 	                         //���ڳ�ʼ��Ϊ115200	
	uart2_init(115200);
	TIM4_Int_Init(9999,7199);
	MPU_Init();	                                     //��ʼ��MPU6050
	MPU6050_EXTI_Init();
	while(mpu_dmp_init())                            //��ʼ��mpu_dmp��
 	{
		printf("Initialization failed��\r\n");		//���ڳ�ʼ��ʧ���ϱ�
	}
	printf("Initialization succeed��\r\n");			//���ڳ�ʼ���ɹ��ϱ�

}

void NVIC_Configuration(void)
{
		NVIC_InitTypeDef  NVIC_InitStructure;	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�

		//////////////////�ⲿ�ж�5���ȼ�����Ҳ����MPU6050 INT���ŵ�����
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;				//ʹ���ⲿ�ж�ͨ��
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;	//��ռ���ȼ�0�� 
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//�����ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
		NVIC_Init(&NVIC_InitStructure); 
	
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4�ж�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;  //��ռ���ȼ�1��
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;  //�����ȼ�3��
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
		NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;//��ռ���ȼ�2
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;		//�����ȼ�2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
		NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
}


