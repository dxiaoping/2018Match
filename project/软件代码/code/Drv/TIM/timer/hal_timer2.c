/*
******************************************************************************
* @file    TIM/TimeBase/main.c 
* @author  ��Ʒ����.Ӳ����
* @version V1.0.0
* @date    2017.09.07
* @brief   ͨ�ö�ʱ��2
******************************************************************************
*/

//Includes
#include "hal_uart4.h"
#include "hal_timer2.h"

#include "keyboard_drv.h"
#include "adc.h"
#include "ultrasonic.h"
#include "segled_16bit.h"
#include "door.h"


TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

NVIC_InitTypeDef NVIC_InitStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
uint16_t capture = 0;
__IO uint16_t CCR1_Val = 6000;//1ms,1KHz
__IO uint16_t CCR2_Val = 600;//0.1ms,10KHz
__IO uint16_t CCR3_Val = 30000;//5ms,200Hz
__IO uint16_t CCR4_Val = 60000;//10ms,100Hz

extern const int32_t mems_alert_timeout;  //300ms
extern const int32_t adc_sample_rate;  //50ms
extern uint8_t MEMS_ALERT;
extern const int32_t ledseg_refresh_tm; //2.5ms , 50Hz
extern const int32_t kbd_scan_tm;  //10ms
extern uint32_t TimeCount;//ϵͳʱ��


/*******************************************************************
*������void TIM2_Init(void)
*���ܣ���ʱ��2��ʼ��
*���룺��
*�������
*����˵������
*******************************************************************/
void TIM2_Init(void)
{
	uint16_t PrescalerValue = 0;
	   
	/* --------------------------- System Clocks Configuration ---------------------*/
	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* --------------------------NVIC Configuration -------------------------------*/
	/* Enable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
  
  
	/* ------------------- TIM2 Configuration:Output Compare Timing Mode ---------*/

	/* 
	SystemCoreClock = SYSCLK_FREQ_72MHz;
	PrescalerValue = (uint16_t) (SystemCoreClock / 6000000) - 1=11;
	TIM2 counter clock at SystemCoreClock/(PrescalerValue+1)=72/(11+1) MHz = 6 MHz
	CC1 update rate = TIM2 counter clock / CCR1_Val = 1KHz
	CC2 update rate = TIM2 counter clock / CCR2_Val = 10KHz
	CC3 update rate = TIM2 counter clock / CCR3_Val = 0.25KHz
	CC4 update rate = TIM2 counter clock / CCR4_Val = 0.1KHz
	*/

	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 6000000) - 1;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Prescaler configuration */
	TIM_PrescalerConfig(TIM2, PrescalerValue, TIM_PSCReloadMode_Immediate);

	/* Output Compare Timing Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM2, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

	/* Output Compare Timing Mode configuration: Channel2 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR2_Val;

	TIM_OC2Init(TIM2, &TIM_OCInitStructure);

	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);

	/* Output Compare Timing Mode configuration: Channel3 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR3_Val;

	TIM_OC3Init(TIM2, &TIM_OCInitStructure);

	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);

	/* Output Compare Timing Mode configuration: Channel4 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR4_Val;

	TIM_OC4Init(TIM2, &TIM_OCInitStructure);

	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Disable);

	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);

	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);

}

/**
  * @brief  This function handles TIM2 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
	static int32_t count = 0;
	
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

		//1ms��ʱ����

		capture = TIM_GetCapture1(TIM2);
		TIM_SetCompare1(TIM2, capture + CCR1_Val);
	}
	else if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);

		//100us��ʱ����

		/***********************����2���ճ�ʱ���̴���************************/
		if(F_UART4_RX_RECEIVING)//���ڽ��մ�������
		{
			UART4_RX_TIMEOUT_COUNT++;//���ڳ�ʱ����
			if(UART4_RX_TIMEOUT_COUNT>3)//���ݽ��ռ������3ms
			{//���ڽ�����ɻ����
				F_UART4_RX_RECEIVING=0;
				UART4_RX_TIMEOUT_COUNT=0;
				F_UART4_RX_FINISH=1;
			}
		}

		capture = TIM_GetCapture2(TIM2);
		TIM_SetCompare2(TIM2, capture + CCR2_Val);
	}
	else if (TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);

		//100ms

		capture = TIM_GetCapture3(TIM2);
		TIM_SetCompare3(TIM2, capture + CCR3_Val);
	}
	else
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);

		//10ms��ʱ����
		TimeCount++;
		TimeCount=(uint32_t)TimeCount;
		if(FlagLockSta)
		{
			LockTimeCount++;
			if(LockTimeCount>600)//����6����Զ��ر�����
			{
				LockTimeCount=0;
				FlagLockSta=0;
			}
		}
		kbd_scan();//����ɨ��
		start_ADC1();//����ADC
		
		count++;
		if(count >= 100)
		{
			count = 0;
			if(MEMS_ALERT < MEMS_ALERT_TRIGER)
			{
				MEMS_ALERT = 0;
			}
		}
		
		capture = TIM_GetCapture4(TIM2);
		TIM_SetCompare4(TIM2, capture + CCR4_Val);
	}
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
