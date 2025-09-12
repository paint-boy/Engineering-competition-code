#ifndef __INTERURP_H__
#define __INTERURP_H__

#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "PID.h"
#include "Encode.h"
#include "usart.h"
#include "stdio.h"
#include "Control_Car.h"

//串口数据存储数组
#define RXbuffer1_size     8
#define RXbuffer2_size     10
#define RXbuffer3_size     30
#define RXbuffer6_size     30
#define RXbuffer4_size     6

extern uint8_t Rxbuffer1[RXbuffer1_size];
extern uint8_t Rxbuffer2[RXbuffer2_size];
extern uint8_t Rxbuffer3[RXbuffer3_size];
extern uint8_t Rxbuffer6[RXbuffer6_size];
extern uint8_t Rxbuffer4[RXbuffer4_size];

extern uint8_t Rxbuffer1_2[RXbuffer1_size];
extern uint8_t Rxbuffer2_2[RXbuffer2_size];
extern uint8_t Rxbuffer3_2[RXbuffer3_size];
extern uint8_t Rxbuffer6_2[RXbuffer6_size];
extern uint8_t Rxbuffer4_2[RXbuffer4_size];





#endif
