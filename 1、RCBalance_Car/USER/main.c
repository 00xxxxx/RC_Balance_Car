#include "sys.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "SysTick.h"
#include "TaskManage.h"-
int main(void) 
{	
	SystemHardwareDriverInit();			//Ӳ����ʼ��
	SysTick_Init(72);
	while(1)   
	{
    TaskCreateFunction();  			//��������
		vTaskStartScheduler();          //�����������

	}
}
