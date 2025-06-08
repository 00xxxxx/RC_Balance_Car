#include "control.h"

#define PI 3.141592654

int Balance_Pwm,Velocity_Pwm,Turn_Pwm;
float Mechanical_angle=0;		 //��е��ֵ��0��
//ֱ����Kp Kd
float balance_UP_KP=400*0.6; 	 
float balance_UP_KD=-0.6*0.6;
//�ٶȻ�Kp Ki
float velocity_KP=-80;
float velocity_KI=-0;	
//ת��Kd Kp
float Turn_Kd,Turn_Kp=35;


float Left_Speed=0;
float Right_Speed=0;

float pitch,roll,yaw; 			//�����ǡ�����ǡ������					  		
short aacx,aacy,aacz;				//���ٶ����ֵ�洢����								
short gyrox,gyroy,gyroz;		//���������ֵ�洢����											 

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

int MAX_ForeAndBack=0;//ǰ��
int MAX_LeftAndRight=0;//����

void Set_MaxSpeed(short receive_val);
void Direction_Mode(short receive_val);
void Cal_Data(void);

//ֱ����PWM,����С���Ƕ�ƽ�⣬�������Ϊ��ǰ�����ǡ�ƽ���0�㡢y������
int Vertical(float Angle,float Mechanical_balance,float Gyro)
{  
	float Bias;
	int balance;
	Bias=Angle-Mechanical_balance;   
	//balance_UP_KP=400*0.6=240; 	 float balance_UP_KD=-0.6*0.6=-0.036;
	//Bias = Angle-Mechanical_balance -> d(Bias)/dt = d(Angle)/t - d(Mechanical_balance)/t -> d(Bias)/dt = d(Angle)/t = Gyro
	balance=balance_UP_KP*Bias+balance_UP_KD*Gyro;  	
	return balance;		//�ǶȲ������pwmֵ
}

//�ٶȻ�pwm,����С���ٶ�ƽ��,���Σ�Ŀ���ٶȡ����ҵ��
int velocity(int Targrt,int encoder_left,int encoder_right)
{  
	static float Velocity,Encoder_Least,Encoder;
	static float Encoder_Integral;
   //=============�ٶ�PI������=======================//	
		Encoder_Least =(encoder_left+encoder_right)-Targrt;                    //===��ȡ�����ٶ�ƫ��==�����ٶȣ����ұ�����֮�ͣ�-Ŀ���ٶȣ��˴�Ϊ�㣩 
		Encoder *= 0.8;		                                                //===һ�׵�ͨ�˲���       
		Encoder += Encoder_Least*0.2;	                                    //===һ�׵�ͨ�˲���    
		Encoder_Integral +=Encoder;                                       //===���ֳ�λ�� ����ʱ�䣺10ms                                     
		if(Encoder_Integral>10000)  	Encoder_Integral=10000;             //===�����޷�
		if(Encoder_Integral<-10000)		Encoder_Integral=-10000;            //===�����޷�	
		/*float velocity_KP=-80;	float velocity_KI=-0;	*/
		Velocity=Encoder*velocity_KP+Encoder_Integral*velocity_KI;        //===�ٶȿ���	
	  if(pitch<-70||pitch>70) 			Encoder_Integral=0;     		//===����رպ��������(�������ǳ�����70��ʱ��С���㵹��,����������ֹ�㵹״̬�»����ۻ����������·���ʱ�����쳣)
	  return Velocity;
}

//ת��PWM������С�����ֲ��٣�����z������
//yaw ��+ ��-
int Turn(int gyro_Z,int RC)
{
	int PWM_out;
	if(((bluetooth_Receive_Val & 0x0020)==0x0020 && (cnt_Rect==(-180) || cnt_Rect==(-270))) 
		|| ((bluetooth_Receive_Val & 0x0010)==0x0010 && (cnt_Tri==(-120) || cnt_Tri==(-240))))
		if((yaw <= 180) && (yaw > 0))
			yaw=yaw-360;	//����任
		else;
	else;

	if		((bluetooth_Receive_Val & 0x0020)==0x0020)	Limit_Angle = yaw - cnt_Rect;	
	else if	((bluetooth_Receive_Val & 0x0010)==0x0010)	Limit_Angle = yaw - cnt_Tri;
	else;
	
	//����
	if((bluetooth_Receive_Val & 0x0020)==0x0020 && Limit_Angle >= -20 && Limit_Angle <= 20)	//ֱ�� 0��20�����
		TIM_Cmd(TIM4, ENABLE);
	else;
	if((bluetooth_Receive_Val & 0x0020)==0x0020 && Limit_Angle >= 70 && Limit_Angle <= 110)	//ת�� 90��20�����
		TIM_Cmd(TIM4, DISABLE);
	else;
	//������
	if((bluetooth_Receive_Val & 0x0010)==0x0010 && Limit_Angle >= -20 && Limit_Angle <= 20 )	//ֱ�� 0��20�����
		TIM_Cmd(TIM4, ENABLE);
	else;
	if((bluetooth_Receive_Val & 0x0010)==0x0010 && Limit_Angle >= 100 && Limit_Angle <= 140)	//ת�� 120��20�����
		TIM_Cmd(TIM4, DISABLE);
	else;
	
	if((bluetooth_Receive_Val & 0x0020)==0x0020 || (bluetooth_Receive_Val & 0x0010)==0x0010)
		PWM_out = Turn_Kd * gyro_Z + Turn_Kp * RC - 80 * Turn_Kd * Limit_Angle ; 
	else
		PWM_out = Turn_Kd * gyro_Z + Turn_Kp * RC;
	return PWM_out;
}




/*�ж��ڲ��ϻ�ȡ�����ǵ����ݡ���ȡ���ת�٣�ͨ����������
�������������յ���ָ�����С���ٶȡ�LED״̬���˶��켣
���С��״̬�����������𡢲������㵹���Ƿ���������Ӧ����*/

void EXTI9_5_IRQHandler(void)
{
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0) 
	{
		EXTI->PR=1<<5; //����жϱ�־λ
		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)
		{ 			
			MPU_Get_Accelerometer(&aacx,&aacy,&aacz);	//�õ����ٶȴ���������
			MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//�õ�����������		
		}
		
		Encoder_Right = Read_Encoder(2);                   
		Encoder_Left  = Read_Encoder(3);
		
	Set_MaxSpeed(bluetooth_Receive_Val);		//�ٶ�����
	Direction_Mode(bluetooth_Receive_Val);	//ת��ģʽ
	if((bluetooth_Receive_Val & 0x0002)==0x0002) led_on();
	else led_off();  
//		if(Flag_Led==1)	led_on();
//		else			led_off();
			
	//ת��Լ��
	//��������ת���ǰ��ʱ��ת��kd=-0.9
	if		((bluetooth_Receive_Val & 0x0080)==0x0000 && (bluetooth_Receive_Val & 0x0040)==0x0000 &&
			(bluetooth_Receive_Val & 0x4000)==0x0000 && (bluetooth_Receive_Val & 0x1000)==0x0000) 		Turn_Kd=-0.9;
	//������ת���ǰ��ʱ��ת��kd=0
	else if	((bluetooth_Receive_Val & 0x0040)==0x0040 ||(bluetooth_Receive_Val & 0x0080)==0x0080 ||
			(bluetooth_Receive_Val & 0x1000)==0x1000 ||(bluetooth_Receive_Val & 0x4000)==0x4000) 		Turn_Kd=0;
	else;

	//���λ�������
	if((bluetooth_Receive_Val & 0x0020)==0x0020 || (bluetooth_Receive_Val & 0x0010)==0x0010) 
	{	
		RectAndTri_Speed=30;	
	}
	else//�رվ��λ������Σ����ر�tim4ʹ�� 
	{	
		RectAndTri_Speed=0;			
		TIM_Cmd(TIM4, DISABLE);
	}
	//���λ������η���ʱ����RectAndTri_Speed��ΪĿ���ٶ�
	if((bluetooth_Receive_Val & 0x0020)==0x0020 || (bluetooth_Receive_Val & 0x0010)==0x0010)
		Velocity_Pwm=velocity(RectAndTri_Speed,Encoder_Left,Encoder_Right);
	//������ForeAndBack_Speed��ΪĿ���ٶ�
	else
		Velocity_Pwm=velocity(ForeAndBack_Speed,Encoder_Left,Encoder_Right);	//�ٶȻ�pwm,����С��ǰ��
		  
	Balance_Pwm =Vertical(pitch,Mechanical_angle,gyroy);		//ֱ����PWM,����С��ƽ�⣬�������Ϊ�����ǡ�xx��y������
	
	if((bluetooth_Receive_Val & 0x0080)==0x0080 || (bluetooth_Receive_Val & 0x0040)==0x0040) 		Turn_Speed = Clockwise_Speed;
	else if((bluetooth_Receive_Val & 0x1000)==0x1000 || (bluetooth_Receive_Val & 0x4000)==0x4000)  	Turn_Speed = LeftAndRight_Speed;
	else	Turn_Speed=0;
	
	Turn_Pwm = Turn(gyroz,Turn_Speed);		//ת��PWM������С�����ֲ��٣�����z������
	
	if((bluetooth_Receive_Val & 0x1000)==0x1000 || (bluetooth_Receive_Val & 0x4000)==0x4000)
	{
		if((bluetooth_Receive_Val & 0x1000)==0x1000)	//����
		{
			Moto1=Balance_Pwm-Velocity_Pwm;
			Moto2=Balance_Pwm-Velocity_Pwm+Turn_Pwm;
		}
		else if((bluetooth_Receive_Val & 0x4000)==0x4000)	//����
		{
			Moto1=Balance_Pwm-Velocity_Pwm-Turn_Pwm;
			Moto2=Balance_Pwm-Velocity_Pwm;
		}
		else;
	}
	else
	{
		Moto1=Balance_Pwm-Velocity_Pwm-Turn_Pwm;		//��������PIDֵ������
		Moto2=Balance_Pwm-Velocity_Pwm+Turn_Pwm;
	}
	
	Xianfu_Pwm();
	
	//������
	if(Flag_PickUp==0 && ((Moto1==7000 && Moto2==7000) || (Moto1==-7000 && Moto2==-7000)))
	{
		cnt1++;
		if(cnt1>200)
			Flag_Stop=1,cnt1=0,Flag_PickUp=1;Flag_Vertical=0;
	}
	else;
	
	//���¼��
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
	
	//�������(���𵽲�������)
	if(Flag_PickUp==1 && roll > 89 && Flag_Vertical==0)//�����ŵ�
	{
		cnt3++;
		if(cnt3 > 300)
				Flag_Stop=0, cnt3=0, Flag_Vertical=1,Flag_PickUp=0;	
	}
	else if(Flag_PickUp==1 && roll < -89 && Flag_Vertical==0)//�����ŵ�
	{
		cnt3++;
		if(cnt3 > 300)
				Flag_Stop=0,cnt3=0,Flag_Vertical=1,Flag_PickUp=0;				
	}
	else;
			
	//�����˶�
	if(Flag_Vertical==1 && roll > 80)
	{
		Moto1=2500, Moto2=0,Flag_Stop=0;
	}
	else if(Flag_Vertical==1 && roll < -80)
	{
		Moto1=0, Moto2=2500,Flag_Stop=0;
	}
	else;
	
	//===��Ǵ���50�ȹرյ��	
	if(pitch<-70 || pitch>70 || Flag_Stop)	    			
	{	                                   															 
		Moto1=0;
		Moto2=0;
	}
	else;
	Set_Pwm(Moto1,Moto2); //����ΪMoto1	
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
		//////////////////////////////////////////ǰ��/////////////////////////////////////////////////////
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
	
	//�޷�
	if		(ForeAndBack_Speed > MAX_ForeAndBack) 		ForeAndBack_Speed = MAX_ForeAndBack;
	else if	(ForeAndBack_Speed < (-MAX_ForeAndBack)) 	ForeAndBack_Speed = (-MAX_ForeAndBack);
	else 												ForeAndBack_Speed = ForeAndBack_Speed;
	
	//////////////////////////////////////////������ת/////////////////////////////////////////////////////
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
	//�޷�
	Clockwise_Speed = Clockwise_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : Clockwise_Speed;
	Clockwise_Speed = Clockwise_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : Clockwise_Speed;

	//////////////////////////////////////////����ת��/////////////////////////////////////////////////////
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
	//�޷�
	LeftAndRight_Speed = LeftAndRight_Speed > MAX_LeftAndRight ? MAX_LeftAndRight : LeftAndRight_Speed;
	LeftAndRight_Speed = LeftAndRight_Speed < (-MAX_LeftAndRight) ? (-MAX_LeftAndRight) : LeftAndRight_Speed;
			
}

