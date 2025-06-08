#ifndef __CONTROL_H
#define __CONTROL_H	 

#include "sys.h"

extern float angle[3];
extern short gyro[3];
extern float Turn_Kd ;

int Vertical(float Angle,float Mechanical_balance,float Gyro);
int velocity(int Targrt,int encoder_left,int encoder_right,int RC);
int Turn(int gyro_Z,int RC);

#endif

