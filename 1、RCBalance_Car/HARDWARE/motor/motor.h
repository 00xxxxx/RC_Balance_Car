#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "sys.h"
#include "PWM.h"

void Motor_Init(void);
void Set_Pwm(int moto1,int moto2);
int myabs(int a);



#define PWMA   TIM1->CCR1  //PA8
#define PWMB   TIM1->CCR4  //PA11

#define AIN2   PBout(12)
#define AIN1   PBout(13)
#define BIN1   PBout(14)
#define BIN2   PBout(15)

#endif
