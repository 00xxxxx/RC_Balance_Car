#include "sys.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "SysTick.h"
#include "TaskManage.h"-
int main(void) 
{	
	SystemHardwareDriverInit();			//硬件初始化
	SysTick_Init(72);
	while(1)   
	{
    TaskCreateFunction();  			//创建任务
		vTaskStartScheduler();          //开启任务调度

	}
}
