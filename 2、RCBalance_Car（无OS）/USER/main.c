#include "sys.h"

/**************************************************************************
调试用上位机变量
**************************************************************************/
unsigned char i;          										 
unsigned char Send_Count; 	

int main(void) 
{	
	SYS_Init();
	
	while(1)   
	{
		if(STA_ECB02)
			OLED_DrawBMP(50,4,82,8,bmpBluetooth);
		else
			OLED_DrawBMP(50,4,82,8,emptyBluetooth);
					
		OLED_ShowString(1,4,"Balance_Car");
		OLED_ShowSignedNum(2,6,pitch*10/10,3);
		OLED_ShowChar(2,10,'.');
		OLED_ShowNum(2,11,(int)(pitch*10)%10,1);	
		
		//printf("bluetooth_Receive_Val = 0x%x\r\n",bluetooth_Receive_Val);
			
		
//		OLED_ShowSignedNum(1,1,MAX_ForeAndBack,3);
//		OLED_ShowSignedNum(1,6,MAX_LeftAndRight,3);
//		OLED_ShowSignedNum(1,11,ForeAndBack_Speed,3);
//		OLED_ShowSignedNum(2,6,LeftAndRight_Speed,3);
//		OLED_ShowSignedNum(2,11,Clockwise_Speed,3);
//		OLED_ShowNum(1,6,flag_if,2);

//		OLED_ShowHexNum(2,1,bluetooth_Receive_Val,4);
		delay_ms(10);
			
/**************************************************************************
调试用上位机
**************************************************************************/
//		DataScope_Get_Channel_Data(yaw, 1 );
//		DataScope_Get_Channel_Data(cnt_Rect, 2 );
//		DataScope_Get_Channel_Data(Limit_Angle, 3 );
//		DataScope_Get_Channel_Data(Moto1, 4 );
//		DataScope_Get_Channel_Data(Moto2, 5 );	
//		Send_Count=DataScope_Data_Generate(5);
//		for( i = 0 ; i < Send_Count; i++) 
//		{
//			while((USART1->SR&0X40)==0);  
//			USART1->DR = DataScope_OutPut_Buffer[i]; 
//		}
//		delay_ms(10);
	}
}







