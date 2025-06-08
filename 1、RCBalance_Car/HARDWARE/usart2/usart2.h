#ifndef __USRAT3_H
#define __USRAT3_H 
#include "sys.h"	  	

#define STA_ECB02 PCin(13) // PC13

extern short bluetooth_Receive_Val;

extern unsigned char bmpBluetooth[];
extern unsigned char emptyBluetooth[];

void BlueTooth_init(u32 bound);
void USART2_IRQHandler(void);

#endif
