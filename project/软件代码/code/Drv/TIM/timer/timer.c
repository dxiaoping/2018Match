#include <stdio.h>
#include "stm32f10x.h"
#include "systick.h"
#include "keyboard_drv.h"
#include "adc.h"
#include "ultrasonic.h"
#include "segled_16bit.h"
#include "timer.h"
//#include "buzzer.h"

extern const int32_t mems_alert_timeout;  //300ms
extern const int32_t adc_sample_rate;  //50ms
extern uint8_t MEMS_ALERT;
extern const int32_t ledseg_refresh_tm; //2.5ms , 50Hz
extern const int32_t kbd_scan_tm;  //10ms


void TIM2_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}

void TIM2_cfg(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	//����TIM2��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	//���½�Timer����Ϊȱʡֵ
	TIM_DeInit(TIM2);

	//�����ڲ�ʱ�Ӹ�TIM2�ṩʱ��Դ
	TIM_InternalClockConfig(TIM2);

	//Ԥ��Ƶϵ��Ϊ36000-1������������ʱ��Ϊ72MHz/3600 = 20kHz
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / TIME2_INPUT_FREQ) -1;

	//����ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	//���ü�����ģʽΪ���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	//���ü��������С��ÿ��1000�����Ͳ���һ�������¼�
	TIM_TimeBaseStructure.TIM_Period = (TIME2_PERIOD_US / TIME2_TICK_US);
	//������Ӧ�õ�TIM2��
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	
	//�������жϱ�־
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	//��ֹARRԤװ�ػ�����
	TIM_ARRPreloadConfig(TIM2, DISABLE);

	//����TIM2���ж�
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

}

void TIM2_test_GPIO_cfg(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                 //ѡ������5
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //���Ƶ�����50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������������
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

int TIM2_init(void)
{
	#if 0
	TIM2_test_GPIO_cfg();
	#endif

	//interrupt init
	TIM2_NVIC_Configuration();

	//tim2 timer init
	TIM2_cfg();
	
	//������ʱ��2
	TIM_Cmd(TIM2,ENABLE);
	
	return 0;
}




//enter ISR per TIME2_PERIOD_US
void TIM2_IRQHandler(void)
{
	static int32_t count = 0;
//	static uint8_t data = 0;
//	static uint16_t buzzerPrescaler = 0;
	
   //����Ƿ�����������¼�
   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
   {
      count++;
	  if(count >= adc_sample_rate * mems_alert_timeout * kbd_scan_tm * ledseg_refresh_tm)
	  	count = 0;
	  
      //���TIM2���жϴ�����λ
      TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);

	  //do keboard scaning
	  if(count % kbd_scan_tm == 0)
	  	kbd_scan();

	  if(count % adc_sample_rate == 0){
		//do ADC per 30ms 
		start_ADC1();
	  }
	  if(count % mems_alert_timeout == 0){
		if(MEMS_ALERT < MEMS_ALERT_TRIGER)
			MEMS_ALERT = 0;
	  }

	  if(count % ledseg_refresh_tm == 0){
	  	refresh_segleds();
	  }

#if 0

      //��PB.5�ܽ������ֵд��ReadValue
      ReadValue = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5);

      if(ReadValue == 0)
      {
         GPIO_SetBits(GPIOB,GPIO_Pin_5);
      }    
      else
      {
         GPIO_ResetBits(GPIOB,GPIO_Pin_5);      
      }
#endif
	  //printf("timeout");
   }
}





/*
void RCC_cfg()
{

      

       //�������״̬����

       ErrorStatus HSEStartUpStatus;

      

       //��RCC�Ĵ�����������ΪĬ��ֵ

       RCC_DeInit();

 

       //���ⲿ����ʱ�Ӿ���

       RCC_HSEConfig(RCC_HSE_ON);

 

       //�ȴ��ⲿ����ʱ�Ӿ�����

       HSEStartUpStatus = RCC_WaitForHSEStartUp();

       if(HSEStartUpStatus == SUCCESS)

       {

              //����AHBʱ��(HCLK)Ϊϵͳʱ��

              RCC_HCLKConfig(RCC_SYSCLK_Div1);

 

              //���ø���AHBʱ��(APB2)ΪHCLKʱ��

              RCC_PCLK2Config(RCC_HCLK_Div1);

 

              //���õ���AHBʱ��(APB1)ΪHCLK��2��Ƶ

              RCC_PCLK1Config(RCC_HCLK_Div2);

             

              //����FLASH������ʱ

              FLASH_SetLatency(FLASH_Latency_2);

 

              //ʹ��Ԥȡָ����

              FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

 

              //����PLLʱ�ӣ�ΪHSE��9��Ƶ 8MHz * 9 = 72MHz

              RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

 

              //ʹ��PLL

              RCC_PLLCmd(ENABLE);

 

              //�ȴ�PLL׼������

              while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

 

              //����PLLΪϵͳʱ��Դ

              RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

 

              //�ж�PLL�Ƿ���ϵͳʱ��

              while(RCC_GetSYSCLKSource() != 0x08);

       }

 

       //����TIM2��ʱ��

       RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

       //����GPIO��ʱ��

       RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

 

}
*/

