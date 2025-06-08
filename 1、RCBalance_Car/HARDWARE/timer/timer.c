#include "timer.h"

int32_t cnt_Rect=0;
int32_t cnt_Tri=0;

/*******************************************************************************
* Function Name  : TIM4_Int_Init
* Description    : TIM4定时器初始化，用于控制小车走特殊形状
* Input          : arr，psc
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ClearFlag(TIM4,TIM_FLAG_Update);
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM4, //TIM4
		TIM_IT_Update ,
		ENABLE  //使能
		);
	
	//TIM_Cmd(TIM4, ENABLE);  //使能TIMx外设
							 
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : TIM4定时器中断
					完成矩形或三角形转向
					1S变换一次方向
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)   
{
	static int i=0;
	static int j=0;
	int angle_Rect[]={0,-90,-180,-270};	//矩形转向数据
	int angle_Tri[]={0,-120,-240};		//三角形转向数据
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM中断源

		i = i % 4;
		cnt_Rect = angle_Rect[i];
		i++;

		j = j % 3;
		cnt_Tri = angle_Tri[j];
		j++;
		
	}
}

