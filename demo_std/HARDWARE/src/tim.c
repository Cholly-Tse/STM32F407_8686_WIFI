#include "tim.h"

void enable_timer(void)
{
	TIM_Cmd(TIM3, ENABLE);
}


void disable_timer(void)
{
	TIM_Cmd(TIM3, DISABLE); 
}

void timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	/*timer2 for uart process*/
   /* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period =TCP_TMR_INTERVAL*10-1;//250ms SYS TIMER
	TIM_TimeBaseStructure.TIM_Prescaler =8400-1;//10k
	TIM_TimeBaseStructure.TIM_ClockDivision =TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	/* Prescaler configuration 0~65535(128M/(per+1))=128M/(127+1)=1M*/
	TIM_PrescalerConfig(TIM3,7199, TIM_PSCReloadMode_Immediate); 
	/* TIM IT enable */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
	TIM_Cmd(TIM3, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
