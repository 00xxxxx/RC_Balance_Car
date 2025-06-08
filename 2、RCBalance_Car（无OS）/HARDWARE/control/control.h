#ifndef __CONTROL_H
#define __CONTROL_H	 

#include "sys.h"

extern int Moto1,Moto2;	
extern float pitch,roll,yaw; 
extern int ForeAndBack_Speed;
extern int LeftAndRight_Speed;
extern int Clockwise_Speed;
extern int Turn_Pwm;
extern int Balance_Pwm;
extern int Velocity_Pwm;
extern int Limit_Angle;

extern int MAX_ForeAndBack;
extern int MAX_LeftAndRight;

void Cal_Data(void);

#endif

