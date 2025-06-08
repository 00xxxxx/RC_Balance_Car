#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LED PBout(0)

void LED_Init(void);
void led_on(void);
void led_off(void);

	 				    
#endif
