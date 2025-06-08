#include "control.h"

#define PI 3.141592654

int Balance_Pwm,Velocity_Pwm,Turn_Pwm;
float Mechanical_angle=0;		 //机械中值：0°
//直立环Kp Kd
float balance_UP_KP=400*0.6; 	 
float balance_UP_KD=-0.6*0.6;
//速度环Kp Ki
float velocity_KP=-80;
float velocity_KI=-0;	
//转向环Kd Kp
float Turn_Kd,Turn_Kp=35;


float Left_Speed=0;
float Right_Speed=0;

float pitch,roll,yaw; 			//俯仰角、横滚角、航向角					  		
short aacx,aacy,aacz;				//加速度输出值存储变量								
short gyrox,gyroy,gyroz;		//陀螺仪输出值存储变量											 

int ForeAndBack_Speed=0;
int RectAndTri_Speed=0;
int LeftAndRight_Speed=0;
int Clockwise_Speed=0;
int Turn_Speed=0;

int Flag_Stop=0;
int Flag_PickUp=0;
int Flag_Vertical=0;
//int	Flag_Led = 0;

int cnt1=0;
int cnt2=0;
int cnt3=0;

int Limit_Angle=0;
int Moto1=0,Moto2=0;

int MAX_ForeAndBack=0;//前后
int MAX_LeftAndRight=0;//左右

void Set_MaxSpeed(short receive_val);
void Direction_Mode(short receive_val);
void Cal_Data(void);

//直立环PWM,控制小车角度平衡，传入参数为当前俯仰角、平衡点0°、y轴数据
int Vertical(float Angle,float Mechanical_balance,float Gyro)
{  
	float Bias;
	int balance;
	Bias=Angle-Mechanical_balance;   
	//balance_UP_KP=400*0.6=240; 	 float balance_UP_KD=-0.6*0.6=-0.036;
	//Bias = Angle-Mechanical_balance -> d(Bias)/dt = d(Angle)/t - d(Mechanical_balance)/t -> d(Bias)/dt = d(Angle)/t = Gyro
	balance=balance_UP_KP*Bias+balance_UP_KD*Gyro;  	
	return balance;		//角度差产生的pwm值
}

//速度环pwm,控制小车速度平衡,传参：目标速度、左右电机
int velocity(int Targrt,int encoder_left,int encoder_right)
{  
	static float Velocity,Encoder_Least,Encoder;
	static float Encoder_Integral;
   //=============速度PI控制器=======================//	
		Encoder_Least =(encoder_left+encoder_right)-Targrt;                    //===获取最新速度偏差==测量速度（左右编码器之和）-目标速度（此处为零） 
		Encoder *= 0.8;		                                                //===一阶低通滤波器       
		Encoder += Encoder_Least*0.2;	                                    //===一阶低通滤波器    
		Encoder_Integral +=Encoder;                                       //===积分出位移 积分时间：10ms                                     
		if(Encoder_Integral>10000)  	Encoder_Integral=10000;             //===积分限幅
		if(Encoder_Integral<-10000)		Encoder_Integral=-10000;            //===积分限幅	
		/*float velocity_KP=-80;	float velocity_KI=-0;	*/
		Velocity=Encoder*velocity_KP+Encoder_Integral*velocity_KI;        //===速度控制	
	  if(pitch<-70||pitch>70) 			Encoder_Integral=0;     		//===电机关闭后清除积分(当俯仰角超过±70°时（小车倾倒）,清零积分项，防止倾倒状态下积分累积，导致重新扶起时控制异常)
	  return Velocity;
}

//转向环PWM，控制小车两轮差速，传入z轴数据
//yaw 左+ 右-
int Turn(int gyro_Z,int RC)
{
	int PWM_out;
	if(((bluetooth_Receive_Val & 0x0020)==0x0020 && (cnt_Rect==(-180) || cnt_Rect==(-270))) 
		|| ((bluetooth_Receive_Val & 0x0010)==0x0010 && (cnt_Tri==(-120) || cnt_Tri==(-240))))
		if((yaw <= 180) && (yaw > 0))
			yaw=yaw-360;	//坐标变换
		else;
	else;

	if		((bluetooth_Receive_Val & 0x0020)==0x0020)	Limit_Angle = yaw - cnt_Rect;	
	else if	((bluetooth_Receive_Val & 0x0010)==0x0010)	Limit_Angle = yaw - cnt_Tri;
	else;
	
	//矩形
	if((bluetooth_Receive_Val & 0x0020)==0x0020 && Limit_Angle >= -20 && Limit_Angle <= 20)	//直行 0±20°误差
		TIM_Cmd(TIM4, ENABLE);
	else;
	if((bluetooth_Receive_Val & 0x0020)==0x0020 && Limit_Angle >= 70 && Limit_Angle <= 110)	//转向 90±20°误差
		TIM_Cmd(TIM4, DISABLE);
	else;
	//三角形
	if((bluetooth_Receive_Val & 0x0010)==0x0010 && Limit_Angle >= -20 && Limit_Angle <= 20 )	//直行 0±20°误差
		TIM_Cmd(TIM4, ENABLE);
	else;
	if((bluetooth_Receive_Val & 0x0010)==0x0010 && Limit_Angle >= 100 && Limit_Angle <= 140)	//转向 120±20°误差
		TIM_Cmd(TIM4, DISABLE);
	else;
	
	if((bluetooth_Receive_Val & 0x0020)==0x0020 || (bluetooth_Receive_Val & 0x0010)==0x0010)
		PWM_out = Turn_Kd * gyro_Z + Turn_Kp * RC - 80 * Turn_Kd * Limit_Angle ; 
	else
		PWM_out = Turn_Kd * gyro_Z + Turn_Kp * RC;
	return PWM_out;
}




/*中断内不断获取陀螺仪的数据、读取电机转速（通过编码器）
并根据蓝牙接收到的指令调整小车速度、LED状态和运动轨迹
检测小车状态，并根据拿起、侧立、倾倒或是放下做出相应动作*/

void EXTI9_5_IRQHandler(void)
{
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0) 
	{
		EXTI->PR=1<<5; //清除中断标志位
		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)
		{ 			
			MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//得到加速度传感器数据
			MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//得到陀螺仪数据		
		}
		
		Encoder_Right = Read_Encoder(2);                   
		Encoder_Left  = Read_Encoder(3);
		
	Set_MaxSpeed(bluetooth_Receive_Val);		//速度限制
	Direction_Mode(bluetooth_Receive_Val);	//转向模式
	if((bluetooth_Receive_Val & 0x0002)==0x0002) led_on();
	else led_off();  
//		if(Flag_Led==1)	led_on();
//		else			led_off();
			
	//转向约束
	//当不发生转向或前后时，转向环kd=-0.9
	if		((bluetooth_Receive_Val & 0x0080)==0x0000 && (bluetooth_Receive_Val & 0x0040)==0x0000 &&
			(bluetooth_Receive_Val & 0x4000)==0x0000 && (bluetooth_Receive_Val & 0x1000)==0x0000) 		Turn_Kd=-0.9;
	//当发生转向或前后时，转向环kd=0
	else if	((bluetooth_Receive_Val & 0x0040)==0x0040 ||(bluetooth_Receive_Val & 0x0080)==0x0080 ||
			(bluetooth_Receive_Val & 0x1000)==0x1000 ||(bluetooth_Receive_Val & 0x4000)==0x4000) 		Turn_Kd=0;
	else;

	//矩形或三角形
	if((bluetooth_Receive_Val & 0x0020)==0x0020 || (bluetooth_Receive_Val & 0x0010)==0x0010) 
	{	
		RectAndTri_Speed=30;	
	}
	else//关闭矩形或三角形，并关闭tim4使能 
	{	
		RectAndTri_Speed=0;			
		TIM_Cmd(TIM4, DISABLE);
	}
	//矩形或三角形发生时则传入RectAndTri_Speed作为目标速度
	if((bluetooth_Receive_Val & 0x0020)==0x0020 || (bluetooth_Receive_Val & 0x0010)==0x0010)
		Velocity_Pwm=velocity(RectAndTri_Speed,Encoder_Left,Encoder_Right);
	//否则传入ForeAndBack_Speed作为目标速度
	else
		Velocity_Pwm=velocity(ForeAndBack_Speed,Encoder_Left,Encoder_Right);	//速度环pwm,控制小车前进
		  
	Balance_Pwm =Vertical(pitch,Mechanical_angle,gyroy);		//直立环PWM,控制小车平衡，传入参数为俯仰角、xx、y轴数据
	
	if((bluetooth_Receive_Val & 0x0080)==0x0080 || (bluetooth_Receive_Val & 0x0040)==0x0040) 		Turn_Speed = Clockwise_Speed;
	else if((bluetooth_Receive_Val & 0x1000)==0x1000 || (bluetooth_Receive_Val & 0x4000)==0x4000)  	Turn_Speed = LeftAndRight_Speed;
	else	Turn_Speed=0;
	
	Turn_Pwm = Turn(gyroz,Turn_Speed);		//转向环PWM，控制小车两轮差速，传入z轴数据
	
	if((bluetooth_Receive_Val & 0x1000)==0x1000 || (bluetooth_Receive_Val & 0x4000)==0x4000)
	{
		if((bluetooth_Receive_Val & 0x1000)==0x1000)	//向左
		{
			Moto1=Balance_Pwm-Velocity_Pwm;
			Moto2=Balance_Pwm-Velocity_Pwm+Turn_Pwm;
		}
		else if((bluetooth_Receive_Val & 0x4000)==0x4000)	//向右
		{
			Moto1=Balance_Pwm-Velocity_Pwm-Turn_Pwm;
			Moto2=Balance_Pwm-Velocity_Pwm;
		}
		else;
	}
	else
	{
		Moto1=Balance_Pwm-Velocity_Pwm-Turn_Pwm;		//将计算后的PID值传入电机
		Moto2=Balance_Pwm-Velocity_Pwm+Turn_Pwm;
	}
	
	Xianfu_Pwm();
	
	//拿起检测
	if(Flag_PickUp==0 && ((Moto1==7000 && Moto2==7000) || (Moto1==-7000 && Moto2==-7000)))
	{
		cnt1++;
		if(cnt1>200)
			Flag_Stop=1,cnt1=0,Flag_PickUp=1;Flag_Vertical=0;
	}
	else;
	
	//放下检测
	if(Flag_PickUp==1)
	{	
		if(pitch < 5 && pitch > -5 && roll > -10 && roll < 10)
		{
			cnt2++;
			if(cnt2>200)
				Flag_Stop=0,cnt2=0,Flag_PickUp=0;			
		}
	}
	else;
	
	//侧立检测(拿起到侧立过程)
	if(Flag_PickUp==1 && roll > 89 && Flag_Vertical==0)//右轮着地
	{
		cnt3++;
		if(cnt3 > 300)
				Flag_Stop=0, cnt3=0, Flag_Vertical=1,Flag_PickUp=0;	
	}
	else if(Flag_PickUp==1 && roll < -89 && Flag_Vertical==0)//左轮着地
	{
		cnt3++;
		if(cnt3 > 300)
				Flag_Stop=0,cnt3=0,Flag_Vertical=1,Flag_PickUp=0;				
	}
	else;
			
	//侧立运动
	if(Flag_Vertical==1 && roll > 80)
	{
		Moto1=2500, Moto2=0,Flag_Stop=0;
	}
	else if(Flag_Vertical==1 && roll < -80)
	{
		Moto1=0, Moto2=2500,Flag_Stop=0;
	}
	else;
	
	//===倾角大于50度关闭电机	
	if(pitch<-70 || pitch>70 || Flag_Stop)	    			
	{	                                   															 
		Moto1=0;
		Moto2=0;
	}
	else;
	Set_Pwm(Moto1,Moto2); //右轮为Moto1	
	}
}

void Set_MaxSpeed(short receive_val)
{

	if((receive_val & 0x0f00)==0x0800)
	{
		MAX_ForeAndBack=60;
		MAX_LeftAndRight=40;
	}
	else if((receive_val & 0x0f00)==0x0400)
	{
		MAX_ForeAndBack=100;
		MAX_LeftAndRight=80;
	}
	else if((receive_val & 0x0f00)==0x0200)
	{
		MAX_ForeAndBack=140;
		MAX_LeftAndRight=150;
	}
	else if(receive_val == 0x0f00)
	{
		MAX_ForeAndBack=0;
		MAX_LeftAndRight=0;
	}		
	else;
	
	
	
}

void Direction_Mode(short receive_val)
{
		//////////////////////////////////////////前后/////////////////////////////////////////////////////
	if((bluetooth_Receive_Val & 0x8000)==0x0000 && (bluetooth_Receive_Val & 0x2000)==0x0000) 
	{
		if(ForeAndBack_Speed>0)
			{
				ForeAndBack_Speed-=2;
				if(ForeAndBack_Speed<0) ForeAndBack_Speed=0;
			}
		else if(ForeAndBack_Speed<0)
			{
				ForeAndBack_Speed+=2;
				if(ForeAndBack_Speed>0) ForeAndBack_Speed=0;
			}
		else;	
	}
	else if((bluetooth_Receive_Val & 0x8000)==0x8000) 			ForeAndBack_Speed+=2;
	else if((bluetooth_Receive_Val & 0x2000)==0x2000) 			ForeAndBack_Speed-=2;
	else;
	
	//限幅
	if		(ForeAndBack_Speed > MAX_ForeAndBack) 		ForeAndBack_Speed = MAX_ForeAndBack;
	else if	(ForeAndBack_Speed < (-MAX_ForeAndBack)) 	ForeAndBack_Speed = (-MAX_ForeAndBack);
	else 												ForeAndBack_Speed = ForeAndBack_Speed;
	
	//////////////////////////////////////////左右旋转/////////////////////////////////////////////////////
	if((bluetooth_Receive_Val & 0x0080)==0x0000 && (bluetooth_Receive_Val & 0x0040)==0x0000)
	{
		if(Clockwise_Speed>0)
		{
			Clockwise_Speed-=2;
			if(Clockwise_Speed<0) Clockwise_Speed=0;
		}
		else if(Clockwise_Speed<0)
		{
			Clockwise_Speed+=2;
			if(Clockwise_Speed>0)	Clockwise_Speed=0;
		}
		else;	
	}
	else if((bluetooth_Receive_Val & 0x0040)==0x0040)  			Clockwise_Speed+=2;
	else if((bluetooth_Receive_Val & 0x0080)==0x0080) 			Clockwise_Speed-=2;
	else;
	//限幅
	Clockwise_Speed = Clockwise_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : Clockwise_Speed;
	Clockwise_Speed = Clockwise_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : Clockwise_Speed;

	//////////////////////////////////////////左右转向/////////////////////////////////////////////////////
	if((bluetooth_Receive_Val & 0x4000)==0x0000 && (bluetooth_Receive_Val & 0x1000)==0x0000)
	{
		if(LeftAndRight_Speed>0)
		{
			LeftAndRight_Speed-=2;
			if(LeftAndRight_Speed<0)LeftAndRight_Speed=0;
		}
		else if(LeftAndRight_Speed<0)
		{
			LeftAndRight_Speed+=2;
			if(LeftAndRight_Speed>0)LeftAndRight_Speed=0;
		}
		else;	
	}
	else if((bluetooth_Receive_Val & 0x1000)==0x1000)   		LeftAndRight_Speed-=2;
	else if((bluetooth_Receive_Val & 0x4000)==0x4000) 			LeftAndRight_Speed+=2;
	else;
	//限幅
	LeftAndRight_Speed = LeftAndRight_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : LeftAndRight_Speed;
	LeftAndRight_Speed = LeftAndRight_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : LeftAndRight_Speed;
			
}

