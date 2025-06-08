#include "TaskManage.h"

#include "sys.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#define Mechanical_angle 			0
#define FORE_ENABLE 				(bluetooth_Receive_Val & 0x8000)==0x8000
#define FORE_DISABLE 				(bluetooth_Receive_Val & 0x8000)==0x0000 
#define BACK_ENABLE 				(bluetooth_Receive_Val & 0x2000)==0x2000
#define BACK_DISABLE 				(bluetooth_Receive_Val & 0x2000)==0x0000

#define LEFT_ENABLE 				(bluetooth_Receive_Val & 0x1000)==0x1000
#define LEFT_DISABLE 				(bluetooth_Receive_Val & 0x1000)==0x0000
#define RIGHT_ENABLE 				(bluetooth_Receive_Val & 0x4000)==0x4000
#define RIGHT_DISABLE 				(bluetooth_Receive_Val & 0x4000)==0x0000

#define Clockwise_ENABLE 			(bluetooth_Receive_Val & 0x0040)==0x0040
#define Clockwise_DISABLE 			(bluetooth_Receive_Val & 0x0040)==0x0000
#define Counterclockwise_ENABLE 	(bluetooth_Receive_Val & 0x0080)==0x0080
#define Counterclockwise_DISABLE 	(bluetooth_Receive_Val & 0x0080)==0x0000

#define Rect_ENABLE 				(bluetooth_Receive_Val & 0x0020)==0x0020
#define Rect_DISABLE 				(bluetooth_Receive_Val & 0x0020)==0x0000
#define Tri_ENABLE 					(bluetooth_Receive_Val & 0x0010)==0x0010
#define Tri_DISABLE 				(bluetooth_Receive_Val & 0x0010)==0x0000


//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		512  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);


#define OLED_TASK_PRIO		2	
#define OLED_STK_SIZE 		128  
TaskHandle_t OledTask_Handler;
void oled_task(void *pvParameters);

#define PID_TASK_PRIO		5	
#define PID_STK_SIZE 		512  
TaskHandle_t PidTask_Handler;
void pid_task(void *pvParameters);

#define BLUETOOTH_TASK_PRIO		5	
#define BLUETOOTH_STK_SIZE 		512 
TaskHandle_t BluetoothTask_Handler;
void bluetooth_task(void *pvParameters);

#define  Reomte_QUEUE_LEN    4   /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define  Reomte_QUEUE_SIZE   4 * sizeof(int)   /* ������ÿ����Ϣ��С���ֽڣ� */
	
QueueHandle_t Reomte_Queue =NULL; 


/*******************************************************************************
* Function Name  : TaskCreateFunction
* Description    : ����������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TaskCreateFunction(void)
{   
	xTaskCreate((TaskFunction_t )start_task,            //������
				(const char*    )"start_task",          //��������
				(uint16_t       )START_STK_SIZE,        //�����ջ��С
				(void*          )NULL,                  //���ݸ��������Ĳ���
				(UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
				(TaskHandle_t*  )&StartTask_Handler);   //������   
}


/*******************************************************************************
* Function Name  : start_task
* Description    : ��ʼ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void start_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
    taskENTER_CRITICAL();           //�����ٽ���
	
	/* ����Reomte_Queue */	
	Reomte_Queue 	= xQueueCreate( (UBaseType_t ) Reomte_QUEUE_LEN,/* ��Ϣ���еĳ��� */
									(UBaseType_t ) Reomte_QUEUE_SIZE);
	if(Reomte_Queue != NULL)
		printf("����Reomte_Queue��Ϣ���гɹ���\r\n");

    //����OLED����
    xReturn = xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OledTask_Handler); 
	if(xReturn==pdPASS)
			printf("OLED TASK SUCCEED!\r\n");
	
    //����PID����
    xReturn = xTaskCreate((TaskFunction_t )pid_task,     
                (const char*    )"pid_task",   
                (uint16_t       )PID_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )PID_TASK_PRIO,
                (TaskHandle_t*  )&PidTask_Handler);
	if(xReturn==pdPASS)
			printf("PID TASK SUCCEED!\r\n");	

    //����BLUETOOTH����
	xReturn = xTaskCreate((TaskFunction_t )bluetooth_task,     
                (const char*    )"bluetooth_task",   
                (uint16_t       )BLUETOOTH_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )BLUETOOTH_TASK_PRIO,
                (TaskHandle_t*  )&BluetoothTask_Handler); 
	if(xReturn==pdPASS)
			printf("BLUETOOTH TASK SUCCEED!\r\n");	
						
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
} 


/*******************************************************************************
* Function Name  : pid_task
* Description    : PID������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void pid_task(void *pvParameters)
{
	int Balance_Pwm,Velocity_Pwm,Turn_Pwm;		//ֱ�������ٶȻ���ת��
	int Encoder_Left,Encoder_Right;						//����������
	int Turn_Speed=0;
	int Moto1=0,Moto2=0;											//����������pwmֵ
	int Remote_Speed[4]={0,0,0,0};						//��ȡң���ٶ�����
	
	int Pickup_Num=0;		//�������
	int Falldown_Num=0;		//���¼���
	int Vertical_Num=0;		//��������
	int Flag_Stop=0;		//ֹͣ��־λ
	int Flag_PickUp=0;		//�����־λ
	int Flag_Vertical=0;	//������־λ
	int Flag_Falldown=0;	//������־λ
	
	while(1)
	{ 
		//���������ң������(ת����)	 ForeAndBack_Speed,RectAndTri_Speed,LeftAndRight_Speed,Clockwise_Speed
		//�˺��������������дӶ����ж�ȡһ����������Ϣ����ȡ�ɹ��Ժ�ͻὫ�����е���������ɾ��
		xQueueReceive(Reomte_Queue,Remote_Speed,0);
			
//			printf("ForeAndBack_Speed = %d\r\n",		Remote_Speed[0]);
//			printf("LeftAndRight_Speed = %d\r\n",		Remote_Speed[1]);
//			printf("Clockwise_Speed = %d\r\n",			Remote_Speed[2]);
//			printf("RectAndTri_Speed = %d\r\n\r\n",		Remote_Speed[3]);	
		
		//1�����Ρ��������ٶȼ���
		Encoder_Right = Read_Encoder(2);	
		Encoder_Left  = Read_Encoder(3);
	
		if(Rect_ENABLE || Tri_ENABLE)
			Velocity_Pwm=velocity(Remote_Speed[3],Encoder_Left,Encoder_Right,0);
		else if(STA_ECB02)
			Velocity_Pwm=velocity(Remote_Speed[0],Encoder_Left,Encoder_Right,0);
		else
			Velocity_Pwm=velocity(Remote_Speed[0],Encoder_Left,Encoder_Right,1);
		
		//2��ת���ٶȼ���
		if		(Counterclockwise_DISABLE && Clockwise_DISABLE &&
				RIGHT_DISABLE && LEFT_DISABLE) 		Turn_Kd=1.5;
		else if	(Clockwise_ENABLE ||Counterclockwise_ENABLE ||
				LEFT_ENABLE ||RIGHT_ENABLE) 		Turn_Kd=0;
		else;  
		if(Counterclockwise_ENABLE || Clockwise_ENABLE) 		Turn_Speed = Remote_Speed[2];
		else if(LEFT_ENABLE || RIGHT_ENABLE)  					Turn_Speed = Remote_Speed[1];
		else	Turn_Speed=0;
		Turn_Pwm = Turn(gyro[2],Turn_Speed);
		
		//3��ƽ���ٶȼ���
		Balance_Pwm =Vertical(angle[0],Mechanical_angle,gyro[1]);
		
		//4�����յ���ٶȼ���
		if(LEFT_ENABLE || RIGHT_ENABLE)
		{
			if(LEFT_ENABLE)						//����
				{
					Moto1=Balance_Pwm-Velocity_Pwm;
					Moto2=Balance_Pwm-Velocity_Pwm+Turn_Pwm;
				}
			else if(RIGHT_ENABLE)			//����
				{
					Moto1=Balance_Pwm-Velocity_Pwm-Turn_Pwm;
					Moto2=Balance_Pwm-Velocity_Pwm;
				}
			else;
		}
		else	
		{
			Moto2=Balance_Pwm-Velocity_Pwm-Turn_Pwm;	
			Moto1=Balance_Pwm-Velocity_Pwm+Turn_Pwm;
		}	
		
		//�޷�
		if(Moto1<-7000 ) Moto1=-7000 ;
		if(Moto1>7000 )  Moto1=7000 ;
		if(Moto2<-7000 ) Moto2=-7000 ;
		if(Moto2>7000 )  Moto2=7000 ;
		
		//������
		if(Flag_PickUp==0 && ((Moto1==7000 && Moto2==7000) || (Moto1==-7000 && Moto2==-7000)))
		{
			Pickup_Num++;
			if(Pickup_Num>200)
			Flag_Stop=1,Pickup_Num=0,Flag_PickUp=1;Flag_Vertical=0;
		}
		else;
		
		//���¼��
		if(Flag_PickUp==1)
		{	
			if(angle[0] < 5 && angle[0] > -5 && angle[1] > -10 && angle[1] < 10)
			{
				Falldown_Num++;
				if(Falldown_Num>200)
					Flag_Stop=0,Falldown_Num=0,Flag_PickUp=0;			
			}
		}
		else;
		
		//�������(���𵽲�������)
		if(Flag_PickUp==1 && angle[1] > 89 && Flag_Vertical==0)//�����ŵ�
		{
			Vertical_Num++;
			if(Vertical_Num > 300)
					Flag_Stop=0, Vertical_Num=0, Flag_Vertical=1,Flag_PickUp=0;	
		}
		else if(Flag_PickUp==1 && angle[1] < -89 && Flag_Vertical==0)//�����ŵ�
		{
			Vertical_Num++;
			if(Vertical_Num > 300)
					Flag_Stop=0,Vertical_Num=0,Flag_Vertical=1,Flag_PickUp=0;				
		}
		else;
				
		//��������
		if(Flag_Vertical==1 && angle[1] > 80)
		{
			Moto1=1800, Moto2=0,Flag_Stop=0;
		}
		else if(Flag_Vertical==1 && angle[1] < -80)
		{
			Moto1=0, Moto2=1800,Flag_Stop=0;
		}
		else;
		
		//�رյ��	
		if(Flag_Stop || Flag_Falldown==1)	    			
		{	                                   															 
			Moto1=0;
			Moto2=0;
		}
		if((bluetooth_Receive_Val & 0x0008)==0x0008) 	
		{
			Set_Pwm(0,0);Flag_Falldown=1;
		}
		else 											
		{
			Set_Pwm(Moto1,Moto2); //����ΪMoto1	
			Flag_Falldown=0;
		}
		
		vTaskDelay(1);
		
	}	
}


/*******************************************************************************
* Function Name  : bluetooth_task
* Description    : BLUETOOTH������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void bluetooth_task(void *pvParameters)
{
	BaseType_t xStatus;
	int MAX_ForeAndBack=0;		//ǰ������ٶ�
	int MAX_LeftAndRight=0;		//��������ٶ�
	int Remote_Speed[4]={0,0,0,0};

	#define ForeAndBack_Speed 			Remote_Speed[0]		//ǰ�������ٶ�
	#define LeftAndRight_Speed 			Remote_Speed[1]		//���ҵ���ٶ�
	#define Clockwise_Speed 			Remote_Speed[2]			
	#define RectAndTri_Speed 			Remote_Speed[3]
	
    while(1)
    {

//		printf("bluetooth_Receive_Val = 0x%x\r\n",bluetooth_Receive_Val);
		if((bluetooth_Receive_Val & 0x0f00)==0x0800)
		{
			MAX_ForeAndBack=20;
			MAX_LeftAndRight=60;
		}
		else if((bluetooth_Receive_Val & 0x0f00)==0x0400)
		{
			MAX_ForeAndBack=30;
			MAX_LeftAndRight=100;
		}
		else if((bluetooth_Receive_Val & 0x0f00)==0x0200)
		{
			MAX_ForeAndBack=40;
			MAX_LeftAndRight=150;
		}
		else if(bluetooth_Receive_Val == 0x0f00)
		{
			MAX_ForeAndBack=0;
			MAX_LeftAndRight=0;
		}		
		else;
		
			//////////////////////////////////////////ǰ��/////////////////////////////////////////////////////
		if(FORE_DISABLE && BACK_DISABLE) 
		{
			if		(ForeAndBack_Speed>0)			{ForeAndBack_Speed-=4;	if(ForeAndBack_Speed<0)	ForeAndBack_Speed=0;}
			else if	(ForeAndBack_Speed<0)			{ForeAndBack_Speed+=4;	if(ForeAndBack_Speed>0)	ForeAndBack_Speed=0;}
			else;	
		}
		else if(FORE_ENABLE) 						ForeAndBack_Speed+=4;
		else if(BACK_ENABLE) 						ForeAndBack_Speed-=4;
		else;
		//�޷�	
		ForeAndBack_Speed = ForeAndBack_Speed > MAX_ForeAndBack ? MAX_ForeAndBack : ForeAndBack_Speed;
		ForeAndBack_Speed = ForeAndBack_Speed < (-MAX_ForeAndBack) ? (-MAX_ForeAndBack) : ForeAndBack_Speed;
		
		//////////////////////////////////////////������ת/////////////////////////////////////////////////////
		if(Counterclockwise_DISABLE && Clockwise_DISABLE)
		{
			if(Clockwise_Speed>0)					{Clockwise_Speed+=4;	if(Clockwise_Speed<0)	Clockwise_Speed=0;}
			else if(Clockwise_Speed<0)				{Clockwise_Speed-=4;	if(Clockwise_Speed>0)	Clockwise_Speed=0;}
			else;	
		}
		else if(Clockwise_ENABLE)  					Clockwise_Speed-=4;
		else if(Counterclockwise_ENABLE) 			Clockwise_Speed+=4;
		else;
		//�޷�
		Clockwise_Speed = Clockwise_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : Clockwise_Speed;
		Clockwise_Speed = Clockwise_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : Clockwise_Speed;

		//////////////////////////////////////////����ת��/////////////////////////////////////////////////////
		if(RIGHT_DISABLE && LEFT_DISABLE)
		{
			if(LeftAndRight_Speed>0)				{LeftAndRight_Speed+=4;		if(LeftAndRight_Speed<0)LeftAndRight_Speed=0;}
			else if(LeftAndRight_Speed<0)			{LeftAndRight_Speed-=4;		if(LeftAndRight_Speed>0)LeftAndRight_Speed=0;}
			else;	
		}
		else if(LEFT_ENABLE)   						LeftAndRight_Speed+=4;
		else if(RIGHT_ENABLE) 						LeftAndRight_Speed-=4;
		else;
		//�޷�
		LeftAndRight_Speed = LeftAndRight_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : LeftAndRight_Speed;
		LeftAndRight_Speed = LeftAndRight_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : LeftAndRight_Speed;
		
		//���λ�������
		if(Rect_ENABLE || Tri_ENABLE) 
		{	
			RectAndTri_Speed=10;	//���λ�������ǰ���ٶ�
		}
		else
		{	
			RectAndTri_Speed=0;			
			TIM_Cmd(TIM4, DISABLE);
		}
		
		//LED����
		if((bluetooth_Receive_Val & 0x0002)==0x0002) led_on();
		else led_off();
//		printf("%d",bluetooth_Receive_Val);
	
		//�������ݵ�PID
		xStatus = xQueueSendToBack(Reomte_Queue,Remote_Speed,0);
		if( xStatus != pdPASS )
		{
			printf( "Could not send to the queue!!!!!!.\r\n" );
		}
		
		vTaskDelay(1);	
    }
}


/*******************************************************************************
* Function Name  : oled_task
* Description    : OLED������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void oled_task(void *pvParameters)
{
    while(1)
    {		
		if(STA_ECB02)
			OLED_DrawBMP(50,4,82,8,bmpBluetooth);
		else
			OLED_DrawBMP(50,4,82,8,emptyBluetooth);		
		
		OLED_ShowString(1,4,"Balance_Car");
		
		OLED_ShowSignedNum(2,6,angle[0]*10/10,3);
		OLED_ShowChar(2,10,'.');
		OLED_ShowNum(2,11,(int)(angle[0]*10)%10,1);
		//printf("OLED������is running!\r\n");
		vTaskDelay(10);
    }
}
