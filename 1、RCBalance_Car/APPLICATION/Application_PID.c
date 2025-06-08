#include "Application_PID.h"

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


float Turn_Kd = 0;													
float angle[3]={0,0,0};		//pitch,roll,yaw
short gyro[3]={0,0,0}; 		//gyrox,gyroy,gyroz


/*******************************************************************************
* Function Name  : Vertical
* Description    : 控制小车角度平衡 	
* Input          : 	Angle: 					小车当前的pitch值
					Mechanical_balance: 	小车平衡点，0°
					Gyro: 					小车当前的gyroy值
* Output         : None
* Return         : 角度差产生的PWM值
*******************************************************************************/
int Vertical(float Angle,float Mechanical_balance,float Gyro)
{  
	float balance_UP_KP=300*0.6; 	//300*0.6	 
	float balance_UP_KD=-1.0*0.6;//-1.5*0.6;	
	float Bias;
	int balance;
	
	Bias=Angle-Mechanical_balance;   
	balance=balance_UP_KP*Bias+balance_UP_KD*Gyro;  	
	return balance;
}

/*******************************************************************************
* Function Name  : velocity
* Description    : 控制小车速度平衡
* Input          : 	Targrt: 		目标速度值，用于遥控控制
					encoder_left: 	左编码器数值
					encoder_right: 	右编码器数值
					RC: 			用于遥控控制 0 or 1		
* Output         : None
* Return         : 编码器产生的PWM值
*******************************************************************************/
int velocity(int Targrt,int encoder_left,int encoder_right,int RC)
{  
	static float Velocity,Encoder_Least,Encoder;
	static float Encoder_Integral;
	float velocity_KP=-150;//-300;
	float velocity_KI=-0;//-0.5;	
 
	Encoder_Least =(encoder_left+encoder_right)-Targrt;                    //===获取最新速度偏差==测量速度（左右编码器之和）-目标速度（此处为零） 
	Encoder *= 0.7;		                                                //===一阶低通滤波器       
	Encoder += Encoder_Least*0.3;	                                    //===一阶低通滤波器    
	Encoder_Integral +=Encoder;                                       //===积分出位移 积分时间：10ms                                     
	if(Encoder_Integral>15000)  	Encoder_Integral=15000;             //===积分限幅
	if(Encoder_Integral<-15000)		Encoder_Integral=-15000;            //===积分限幅	
	Velocity=Encoder*velocity_KP+Encoder_Integral*velocity_KI*RC;        //===速度控制	
	if(angle[0]<-70 || angle[0]>70 || STA_ECB02 || encoder_left>70 || encoder_left<-70) 			
		Encoder_Integral=0;     						
	return Velocity;
}
//yaw 左+ 右-
/*******************************************************************************
* Function Name  : Turn
* Description    : 控制小车转向平衡
* Input          : 	gyro_Z: gyroz
					RC: 	转向速度
* Output         : None
* Return         : 偏角产生的PWM值
*******************************************************************************/
int Turn(int gyro_Z,int RC)
{
	float Turn_Kp=35;		//35
	int PWM_out;
	int Limit_Angle=0;
	
	if((Rect_ENABLE && (cnt_Rect==(-180) || cnt_Rect==(-270))) 
		|| (Tri_ENABLE && (cnt_Tri==(-120) || cnt_Tri==(-240))))
		if((angle[2] <= 180) && (angle[2] > 0))
			angle[2]=angle[2]-360;	//坐标变换
		else;
	else;

	if		(Rect_ENABLE)	Limit_Angle = angle[2] - cnt_Rect;	
	else if	(Tri_ENABLE)	Limit_Angle = angle[2] - cnt_Tri;
	else;
	
	//矩形
	if(Rect_ENABLE && Limit_Angle >= -20 && Limit_Angle <= 20)	//直行 0±20°误差
		TIM_Cmd(TIM4, ENABLE);
	else;
	if(Rect_ENABLE && Limit_Angle >= 70 && Limit_Angle <= 110)	//转向 90±20°误差
		TIM_Cmd(TIM4, DISABLE);
	else;
	//三角形
	if(Tri_ENABLE && Limit_Angle >= -20 && Limit_Angle <= 20 )	//直行 0±20°误差
		TIM_Cmd(TIM4, ENABLE);
	else;
	if(Tri_ENABLE && Limit_Angle >= 100 && Limit_Angle <= 140)	//转向 120±20°误差
		TIM_Cmd(TIM4, DISABLE);
	else;
	
	if(Rect_ENABLE || Tri_ENABLE)
		PWM_out = Turn_Kd * gyro_Z + Turn_Kp * RC - 80 * Turn_Kd * Limit_Angle ; 
	else
		PWM_out = Turn_Kd * gyro_Z + Turn_Kp * RC;
	return PWM_out;
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : MPU6050中断处理函数，读取姿态数据
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0) 
	{
		EXTI->PR=1<<5; //清楚中断标志位
		if(mpu_dmp_get_data(&angle[0],&angle[1],&angle[2])==0)
		{
			MPU_Get_Gyroscope(&gyro[0],&gyro[1],&gyro[2]);
		}	
	}	
}

