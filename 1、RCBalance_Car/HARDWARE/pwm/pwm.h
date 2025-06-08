#ifndef _PWM_H_
#define _PWM_H_

#include "sys.h"

void PWM_Init(void);
void PWM_SetCompare1(uint16_t Compare);
void PWM_SetCompare4(uint16_t Compare);

#endif
