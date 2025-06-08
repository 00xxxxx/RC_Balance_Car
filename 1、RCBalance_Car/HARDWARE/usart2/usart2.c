#include "usart2.h"
#include "bmp.h"

short bluetooth_Receive_Val=0;

/*******************************************************************************
* Function Name  : BlueTooth_init
* Description    : ������ʼ�� 	PA2--USART2_TX--RXD_ECB02
								PA3--USART2_RX--TXD_ECB02
								STA_ECB02--PC13
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BlueTooth_init(u32 bound)
{  	 
	 //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��UGPIOBʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��UGPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//ʹ��USART2ʱ��
	
	//USART2_TX  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PB.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//USART2_RX	  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PB3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//STA_ECB02  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//PC13
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure);     //��ʼ������2
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2
}


/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : �����ж�
							0xffff->1111 1111 1111 1111
							1000 0000 0000 0000   0x8000 ǰ
							0100 0000 0000 0000   0x4000 ��
							0010 0000 0000 0000   0x2000 ��
							0001 0000 0000 0000   0x1000 ��
							0000 0000 0000 0000   0x0000 ֹͣ
							0000 1000 0000 0000   0x0800 һ��
							0000 0100 0000 0000   0x0400 ����
							0000 0010 0000 0000   0x0200 ����
							0000 0000 1000 0000   0x0080 ����
							0000 0000 0100 0000   0x0040 ����
							0000 0000 0010 0000   0x0020 Rect
							0000 0000 0001 0000   0x0010 Tri
							0000 0000 0000 1000   0x0008 FALL
							0000 0000 0000 0010   0x0002 LED
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{	
	static int uart_receive=0;//����������ر���
	static short temp_speed=0x0000;
	static short temp_led=0x0000;
	static short temp_xuan=0x0000;

	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //���յ�����
	{	  	
		uart_receive=USART_ReceiveData(USART2); 
		if(uart_receive==0x01)	  		bluetooth_Receive_Val=0x8000+temp_speed+temp_led; //ǰ led/speed/directionͬʱ����
		else if(uart_receive==0x02)		bluetooth_Receive_Val=0x4000+temp_speed+temp_led;	//��
		else if(uart_receive==0x03)		bluetooth_Receive_Val=0x2000+temp_speed+temp_led;	//��	
		else if(uart_receive==0x04)	  	bluetooth_Receive_Val=0x1000+temp_speed+temp_led;	//��		
		
		else if(uart_receive==0x06)		temp_speed=0x0800,bluetooth_Receive_Val=temp_speed;	//һ��
		else if(uart_receive==0x07)	 	temp_speed=0x0400,bluetooth_Receive_Val=temp_speed;	//����
		else if(uart_receive==0x08)		temp_speed=0x0200,bluetooth_Receive_Val=temp_speed;	//����
		else if(uart_receive==0x88)		temp_speed=0x0000,bluetooth_Receive_Val=temp_speed;	//�յ�
		
		else if(uart_receive==0x11)		temp_xuan=0x0080,bluetooth_Receive_Val = temp_speed+temp_xuan;	//����	led/speed/directionͬʱ����
		else if(uart_receive==0x12)		temp_xuan=0x0000,bluetooth_Receive_Val =temp_speed+temp_xuan;	
		else if(uart_receive==0x13)		temp_xuan=0x0040,bluetooth_Receive_Val =temp_speed+temp_xuan;	//����	
		else if(uart_receive==0x14)		temp_xuan=0x0000,bluetooth_Receive_Val =temp_speed+temp_xuan;				
		
		else if(uart_receive==0x15)		temp_led=0x0002,bluetooth_Receive_Val=temp_speed + temp_xuan + 0x0002;	//LED
		else if(uart_receive==0x16)		temp_led=0x0000,bluetooth_Receive_Val=temp_speed + temp_xuan;
		
		else if(uart_receive==0x17)		bluetooth_Receive_Val=temp_led + temp_speed + 0x0020;	//Rect
		else if(uart_receive==0x18)		bluetooth_Receive_Val=temp_led + temp_speed;
		else if(uart_receive==0x19)		bluetooth_Receive_Val=temp_led + temp_speed + 0x0010;	//Tri
		else if(uart_receive==0x1a)		bluetooth_Receive_Val=temp_led + temp_speed;
		else if(uart_receive==0x1b)		bluetooth_Receive_Val=0x0008;
		else if(uart_receive==0x1c)		bluetooth_Receive_Val=0x0000;
		else if(uart_receive==0x00)		bluetooth_Receive_Val=0x0000;
//		else							bluetooth_Receive_Val=temp_speed + temp_led;
		else;

	}  											 
} 



