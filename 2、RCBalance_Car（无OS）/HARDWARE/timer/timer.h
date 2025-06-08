#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h" 

extern int32_t cnt_Rect;
extern int32_t cnt_Tri;
void TIM4_Int_Init(u16 arr,u16 psc);
void TIM4_IRQHandler(void);
 
#endif
