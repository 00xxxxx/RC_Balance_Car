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
	 //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能UGPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能UGPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2时钟
	
	//USART2_TX  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PB.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//USART2_RX	  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PB3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//STA_ECB02  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//PC13
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure);     //初始化串口2
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_Cmd(USART2, ENABLE);                    //使能串口2
}

/**************************************************************************
函数功能：串口2接收中断

0x00:停      0x01:前       0x02:右        0x03:后      0x04:左
0x88:停      0x06:一档     0x07:二档      0x08:三档
0x11:左旋开  0x12:左旋关   0x13:右旋开    0x14:右旋关
0x15:led开   0x16:led关    0x17:rect开    0x18:rect关  0x19:tri开    0x1a:tri关

short bluetooth_Receive_Val=0;
0xffff->1111 1111 1111 1111

1000 0000 0000 0000   0x8000 前
0100 0000 0000 0000   0x4000 右
0010 0000 0000 0000   0x2000 后
0001 0000 0000 0000   0x1000 左

0000 0000 0000 0000   0x0000 停止
0000 1000 0000 0000   0x0800 一档
0000 0100 0000 0000   0x0400 二档
0000 0010 0000 0000   0x0200 三档

0000 0000 1000 0000   0x0080 左旋
0000 0000 0100 0000   0x0040 右旋
0000 0000 0010 0000   0x0020 Rect
0000 0000 0001 0000   0x0010 Tri

0000 0000 0000 1000   0x0008 FALL
0000 0000 0000 0010   0x0002 LED
**************************************************************************/
void USART2_IRQHandler(void)
{	
	int uart_receive;//蓝牙接收相关变量
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收到数据
	{	  
		
		static short temp_speed=0x0000;
		static short temp_led=0x0000;
		static short temp_xuan=0x0000;
		
		uart_receive=USART_ReceiveData(USART2); 
		
		if(uart_receive ==0x01)	  		bluetooth_Receive_Val=temp_led + temp_speed + 0x8000,flag_if=1; //前 led/speed/direction同时存在
		else if(uart_receive==0x02)		bluetooth_Receive_Val=temp_led + temp_speed + 0x4000,flag_if=2;	//右
		else if(uart_receive==0x03)		bluetooth_Receive_Val=temp_led + temp_speed + 0x2000,flag_if=3;	//后	
		else if(uart_receive==0x04)	  bluetooth_Receive_Val=temp_led + temp_speed + 0x1000,flag_if=4;	//左		
		
		else if(uart_receive==0x06)		temp_speed=0x0800,bluetooth_Receive_Val=temp_speed,flag_if=5;	//一档
		else if(uart_receive==0x07)	 	temp_speed=0x0400,bluetooth_Receive_Val=temp_speed,flag_if=6;	//二档
		else if(uart_receive==0x08)		temp_speed=0x0200,bluetooth_Receive_Val=temp_speed,flag_if=7;	//三档
		else if(uart_receive==0x88)		temp_speed=0x0f00,bluetooth_Receive_Val=temp_speed,flag_if=8;	//空挡
		
		else if(uart_receive==0x11)		temp_xuan=0x0080,bluetooth_Receive_Val=temp_led + temp_speed + temp_xuan,flag_if=9;	//左旋	led/speed/direction同时存在
		else if(uart_receive==0x12)		temp_xuan=0x0000,bluetooth_Receive_Val=temp_led + temp_speed,flag_if=10;	//
		else if(uart_receive==0x13)		temp_xuan=0x0040,bluetooth_Receive_Val=temp_led + temp_speed + temp_xuan,flag_if=11;	//右旋	
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



