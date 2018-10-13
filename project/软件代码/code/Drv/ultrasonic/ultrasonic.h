
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#define TIM4_SRC	100000

typedef void (*ULTRASONIC_CALLBACK_FUNC)(uint32_t ultrasonic_new_dist);


//��ʼ��TIM3 channel3Ϊ���벶׽���г���������
void TIM4_ch3_input_capture_ultrasonic_init(ULTRASONIC_CALLBACK_FUNC ult_update_notify_cb);
//��ȡ�������⵽����(��λΪ:ms)
uint16_t read_ultrasonic_dist(void);


#endif

