#include "usart2.h"
#include "bmp.h"
 /**************************************************************************

//PA2--USART2_TX--RXD_ECB02
//PA3--USART2_RX--TXD_ECB02
//STA_ECB02--PC13

**************************************************************************/

short bluetooth_Receive_Val=0x0000;
int flag_if=99;

void uart2_init(u32 bound)
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

/**************************************************************************
�������ܣ�����2�����ж�

0x00:ͣ      0x01:ǰ       0x02:��        0x03:��      0x04:��
0x88:ͣ      0x06:һ��     0x07:����      0x08:����
0x11:������  0x12:������   0x13:������    0x14:������
0x15:led��   0x16:led��    0x17:rect��    0x18:rect��  0x19:tri��    0x1a:tri��

short bluetooth_Receive_Val=0;
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
**************************************************************************/
void USART2_IRQHandler(void)
{	
	int uart_receive;//����������ر���
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //���յ�����
	{	  
		
		static short temp_speed=0x0000;
		static short temp_led=0x0000;
		static short temp_xuan=0x0000;
		
		uart_receive=USART_ReceiveData(USART2); 
		
		if(uart_receive ==0x01)	  		bluetooth_Receive_Val=temp_led + temp_speed + 0x8000,flag_if=1; //ǰ led/speed/directionͬʱ����
		else if(uart_receive==0x02)		bluetooth_Receive_Val=temp_led + temp_speed + 0x4000,flag_if=2;	//��
		else if(uart_receive==0x03)		bluetooth_Receive_Val=temp_led + temp_speed + 0x2000,flag_if=3;	//��	
		else if(uart_receive==0x04)	  bluetooth_Receive_Val=temp_led + temp_speed + 0x1000,flag_if=4;	//��		
		
		else if(uart_receive==0x06)		temp_speed=0x0800,bluetooth_Receive_Val=temp_speed,flag_if=5;	//һ��
		else if(uart_receive==0x07)	 	temp_speed=0x0400,bluetooth_Receive_Val=temp_speed,flag_if=6;	//����
		else if(uart_receive==0x08)		temp_speed=0x0200,bluetooth_Receive_Val=temp_speed,flag_if=7;	//����
		else if(uart_receive==0x88)		temp_speed=0x0f00,bluetooth_Receive_Val=temp_speed,flag_if=8;	//�յ�
		
		else if(uart_receive==0x11)		temp_xuan=0x0080,bluetooth_Receive_Val=temp_led + temp_speed + temp_xuan,flag_if=9;	//����	led/speed/directionͬʱ����
		else if(uart_receive==0x12)		temp_xuan=0x0000,bluetooth_Receive_Val=temp_led + temp_speed,flag_if=10;	//
		else if(uart_receive==0x13)		temp_xuan=0x0040,bluetooth_Receive_Val=temp_led + temp_speed + temp_xuan,flag_if=11;	//����	
		else if(uart_receive==0x14)		temp_xuan=0x0000,bluetooth_Receive_Val=temp_led + temp_speed,flag_if=12;	//			
		
		else if(uart_receive==0x15)		temp_led=0x0002,bluetooth_Receive_Val=bluetooth_Receive_Val + 0x0002,flag_if=13;	//LED
		else if(uart_receive==0x16)		temp_led=0x0000,bluetooth_Receive_Val=bluetooth_Receive_Val - 0x0002,flag_if=14;
		
		else if(uart_receive==0x17)		bluetooth_Receive_Val=temp_led + temp_speed + 0x0020,flag_if=15;	//Rect
		else if(uart_receive==0x18)		bluetooth_Receive_Val=temp_led + temp_speed,flag_if=16;
		else if(uart_receive==0x19)		bluetooth_Receive_Val=temp_led + temp_speed + 0x0010,flag_if=17;	//Tri
		else if(uart_receive==0x1a)		bluetooth_Receive_Val=temp_led + temp_speed,flag_if=18;
		else if(uart_receive==0x00)		bluetooth_Receive_Val=(bluetooth_Receive_Val & 0x0f00)+temp_led,flag_if=0;
		else							bluetooth_Receive_Val=0x0000,flag_if=19;
			
		//printf("uart_receive = %d\r\n",uart_receive);
		//printf("bluetooth_Receive_Val = 0x%x\r\n",bluetooth_Receive_Val);
		
	}  											 
} 



