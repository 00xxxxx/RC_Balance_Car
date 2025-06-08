#include "timer.h"

int32_t cnt_Rect=0;
int32_t cnt_Tri=0;

/*******************************************************************************
* Function Name  : TIM4_Int_Init
* Description    : TIM4��ʱ����ʼ�������ڿ���С����������״
* Input          : arr��psc
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ClearFlag(TIM4,TIM_FLAG_Update);
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM4, //TIM4
		TIM_IT_Update ,
		ENABLE  //ʹ��
		);
	
	//TIM_Cmd(TIM4, ENABLE);  //ʹ��TIMx����
							 
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : TIM4��ʱ���ж�
					��ɾ��λ�������ת��
					1S�任һ�η���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)   
{
	static int i=0;
	static int j=0;
	int angle_Rect[]={0,-90,-180,-270};	//����ת������
	int angle_Tri[]={0,-120,-240};		//������ת������
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM�ж�Դ

		i = i % 4;
		cnt_Rect = angle_Rect[i];
		i++;

		j = j % 3;
		cnt_Tri = angle_Tri[j];
		j++;
		
	}
}

