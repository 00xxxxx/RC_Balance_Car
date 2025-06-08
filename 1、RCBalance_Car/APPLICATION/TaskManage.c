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


//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		512  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
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

#define  Reomte_QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  Reomte_QUEUE_SIZE   4 * sizeof(int)   /* 队列中每个消息大小（字节） */
	
QueueHandle_t Reomte_Queue =NULL; 


/*******************************************************************************
* Function Name  : TaskCreateFunction
* Description    : 创建任务函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TaskCreateFunction(void)
{   
	xTaskCreate((TaskFunction_t )start_task,            //任务函数
				(const char*    )"start_task",          //任务名称
				(uint16_t       )START_STK_SIZE,        //任务堆栈大小
				(void*          )NULL,                  //传递给任务函数的参数
				(UBaseType_t    )START_TASK_PRIO,       //任务优先级
				(TaskHandle_t*  )&StartTask_Handler);   //任务句柄   
}


/*******************************************************************************
* Function Name  : start_task
* Description    : 开始任务函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void start_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
    taskENTER_CRITICAL();           //进入临界区
	
	/* 创建Reomte_Queue */	
	Reomte_Queue 	= xQueueCreate( (UBaseType_t ) Reomte_QUEUE_LEN,/* 消息队列的长度 */
									(UBaseType_t ) Reomte_QUEUE_SIZE);
	if(Reomte_Queue != NULL)
		printf("创建Reomte_Queue消息队列成功！\r\n");

    //创建OLED任务
    xReturn = xTaskCreate((TaskFunction_t )oled_task,     
                (const char*    )"oled_task",   
                (uint16_t       )OLED_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )OLED_TASK_PRIO,
                (TaskHandle_t*  )&OledTask_Handler); 
	if(xReturn==pdPASS)
			printf("OLED TASK SUCCEED!\r\n");
	
    //创建PID任务
    xReturn = xTaskCreate((TaskFunction_t )pid_task,     
                (const char*    )"pid_task",   
                (uint16_t       )PID_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )PID_TASK_PRIO,
                (TaskHandle_t*  )&PidTask_Handler);
	if(xReturn==pdPASS)
			printf("PID TASK SUCCEED!\r\n");	

    //创建BLUETOOTH任务
	xReturn = xTaskCreate((TaskFunction_t )bluetooth_task,     
                (const char*    )"bluetooth_task",   
                (uint16_t       )BLUETOOTH_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )BLUETOOTH_TASK_PRIO,
                (TaskHandle_t*  )&BluetoothTask_Handler); 
	if(xReturn==pdPASS)
			printf("BLUETOOTH TASK SUCCEED!\r\n");	
						
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
} 


/*******************************************************************************
* Function Name  : pid_task
* Description    : PID任务函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void pid_task(void *pvParameters)
{
	int Balance_Pwm,Velocity_Pwm,Turn_Pwm;		//直立环，速度环，转向环
	int Encoder_Left,Encoder_Right;						//电机脉冲采样
	int Turn_Speed=0;
	int Moto1=0,Moto2=0;											//输出到电机的pwm值
	int Remote_Speed[4]={0,0,0,0};						//获取遥控速度数据
	
	int Pickup_Num=0;		//拿起计数
	int Falldown_Num=0;		//放下计数
	int Vertical_Num=0;		//侧立计数
	int Flag_Stop=0;		//停止标志位
	int Flag_PickUp=0;		//拿起标志位
	int Flag_Vertical=0;	//侧立标志位
	int Flag_Falldown=0;	//跌倒标志位
	
	while(1)
	{ 
		//蓝牙传输的遥控数据(转换后)	 ForeAndBack_Speed,RectAndTri_Speed,LeftAndRight_Speed,Clockwise_Speed
		//此函数用于在任务中从队列中读取一条（请求）消息，读取成功以后就会将队列中的这条数据删除
		xQueueReceive(Reomte_Queue,Remote_Speed,0);
			
//			printf("ForeAndBack_Speed = %d\r\n",		Remote_Speed[0]);
//			printf("LeftAndRight_Speed = %d\r\n",		Remote_Speed[1]);
//			printf("Clockwise_Speed = %d\r\n",			Remote_Speed[2]);
//			printf("RectAndTri_Speed = %d\r\n\r\n",		Remote_Speed[3]);	
		
		//1、矩形、三角形速度计算
		Encoder_Right = Read_Encoder(2);	
		Encoder_Left  = Read_Encoder(3);
	
		if(Rect_ENABLE || Tri_ENABLE)
			Velocity_Pwm=velocity(Remote_Speed[3],Encoder_Left,Encoder_Right,0);
		else if(STA_ECB02)
			Velocity_Pwm=velocity(Remote_Speed[0],Encoder_Left,Encoder_Right,0);
		else
			Velocity_Pwm=velocity(Remote_Speed[0],Encoder_Left,Encoder_Right,1);
		
		//2、转向速度计算
		if		(Counterclockwise_DISABLE && Clockwise_DISABLE &&
				RIGHT_DISABLE && LEFT_DISABLE) 		Turn_Kd=1.5;
		else if	(Clockwise_ENABLE ||Counterclockwise_ENABLE ||
				LEFT_ENABLE ||RIGHT_ENABLE) 		Turn_Kd=0;
		else;  
		if(Counterclockwise_ENABLE || Clockwise_ENABLE) 		Turn_Speed = Remote_Speed[2];
		else if(LEFT_ENABLE || RIGHT_ENABLE)  					Turn_Speed = Remote_Speed[1];
		else	Turn_Speed=0;
		Turn_Pwm = Turn(gyro[2],Turn_Speed);
		
		//3、平衡速度计算
		Balance_Pwm =Vertical(angle[0],Mechanical_angle,gyro[1]);
		
		//4、最终电机速度计算
		if(LEFT_ENABLE || RIGHT_ENABLE)
		{
			if(LEFT_ENABLE)						//向左
				{
					Moto1=Balance_Pwm-Velocity_Pwm;
					Moto2=Balance_Pwm-Velocity_Pwm+Turn_Pwm;
				}
			else if(RIGHT_ENABLE)			//向右
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
		
		//限幅
		if(Moto1<-7000 ) Moto1=-7000 ;
		if(Moto1>7000 )  Moto1=7000 ;
		if(Moto2<-7000 ) Moto2=-7000 ;
		if(Moto2>7000 )  Moto2=7000 ;
		
		//拿起检测
		if(Flag_PickUp==0 && ((Moto1==7000 && Moto2==7000) || (Moto1==-7000 && Moto2==-7000)))
		{
			Pickup_Num++;
			if(Pickup_Num>200)
			Flag_Stop=1,Pickup_Num=0,Flag_PickUp=1;Flag_Vertical=0;
		}
		else;
		
		//放下检测
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
		
		//侧立检测(拿起到侧立过程)
		if(Flag_PickUp==1 && angle[1] > 89 && Flag_Vertical==0)//右轮着地
		{
			Vertical_Num++;
			if(Vertical_Num > 300)
					Flag_Stop=0, Vertical_Num=0, Flag_Vertical=1,Flag_PickUp=0;	
		}
		else if(Flag_PickUp==1 && angle[1] < -89 && Flag_Vertical==0)//左轮着地
		{
			Vertical_Num++;
			if(Vertical_Num > 300)
					Flag_Stop=0,Vertical_Num=0,Flag_Vertical=1,Flag_PickUp=0;				
		}
		else;
				
		//侧立动作
		if(Flag_Vertical==1 && angle[1] > 80)
		{
			Moto1=1800, Moto2=0,Flag_Stop=0;
		}
		else if(Flag_Vertical==1 && angle[1] < -80)
		{
			Moto1=0, Moto2=1800,Flag_Stop=0;
		}
		else;
		
		//关闭电机	
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
			Set_Pwm(Moto1,Moto2); //右轮为Moto1	
			Flag_Falldown=0;
		}
		
		vTaskDelay(1);
		
	}	
}


/*******************************************************************************
* Function Name  : bluetooth_task
* Description    : BLUETOOTH任务函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void bluetooth_task(void *pvParameters)
{
	BaseType_t xStatus;
	int MAX_ForeAndBack=0;		//前后最大速度
	int MAX_LeftAndRight=0;		//左右最大速度
	int Remote_Speed[4]={0,0,0,0};

	#define ForeAndBack_Speed 			Remote_Speed[0]		//前进后退速度
	#define LeftAndRight_Speed 			Remote_Speed[1]		//左右电机速度
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
		
			//////////////////////////////////////////前后/////////////////////////////////////////////////////
		if(FORE_DISABLE && BACK_DISABLE) 
		{
			if		(ForeAndBack_Speed>0)			{ForeAndBack_Speed-=4;	if(ForeAndBack_Speed<0)	ForeAndBack_Speed=0;}
			else if	(ForeAndBack_Speed<0)			{ForeAndBack_Speed+=4;	if(ForeAndBack_Speed>0)	ForeAndBack_Speed=0;}
			else;	
		}
		else if(FORE_ENABLE) 						ForeAndBack_Speed+=4;
		else if(BACK_ENABLE) 						ForeAndBack_Speed-=4;
		else;
		//限幅	
		ForeAndBack_Speed = ForeAndBack_Speed > MAX_ForeAndBack ? MAX_ForeAndBack : ForeAndBack_Speed;
		ForeAndBack_Speed = ForeAndBack_Speed < (-MAX_ForeAndBack) ? (-MAX_ForeAndBack) : ForeAndBack_Speed;
		
		//////////////////////////////////////////左右旋转/////////////////////////////////////////////////////
		if(Counterclockwise_DISABLE && Clockwise_DISABLE)
		{
			if(Clockwise_Speed>0)					{Clockwise_Speed+=4;	if(Clockwise_Speed<0)	Clockwise_Speed=0;}
			else if(Clockwise_Speed<0)				{Clockwise_Speed-=4;	if(Clockwise_Speed>0)	Clockwise_Speed=0;}
			else;	
		}
		else if(Clockwise_ENABLE)  					Clockwise_Speed-=4;
		else if(Counterclockwise_ENABLE) 			Clockwise_Speed+=4;
		else;
		//限幅
		Clockwise_Speed = Clockwise_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : Clockwise_Speed;
		Clockwise_Speed = Clockwise_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : Clockwise_Speed;

		//////////////////////////////////////////左右转向/////////////////////////////////////////////////////
		if(RIGHT_DISABLE && LEFT_DISABLE)
		{
			if(LeftAndRight_Speed>0)				{LeftAndRight_Speed+=4;		if(LeftAndRight_Speed<0)LeftAndRight_Speed=0;}
			else if(LeftAndRight_Speed<0)			{LeftAndRight_Speed-=4;		if(LeftAndRight_Speed>0)LeftAndRight_Speed=0;}
			else;	
		}
		else if(LEFT_ENABLE)   						LeftAndRight_Speed+=4;
		else if(RIGHT_ENABLE) 						LeftAndRight_Speed-=4;
		else;
		//限幅
		LeftAndRight_Speed = LeftAndRight_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : LeftAndRight_Speed;
		LeftAndRight_Speed = LeftAndRight_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : LeftAndRight_Speed;
		
		//矩形或三角形
		if(Rect_ENABLE || Tri_ENABLE) 
		{	
			RectAndTri_Speed=10;	//矩形或三角形前进速度
		}
		else
		{	
			RectAndTri_Speed=0;			
			TIM_Cmd(TIM4, DISABLE);
		}
		
		//LED控制
		if((bluetooth_Receive_Val & 0x0002)==0x0002) led_on();
		else led_off();
//		printf("%d",bluetooth_Receive_Val);
	
		//传输数据到PID
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
* Description    : OLED任务函数
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
		//printf("OLED任务函数is running!\r\n");
		vTaskDelay(10);
    }
}
