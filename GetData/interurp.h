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
#define RXbuffer3_size     44
#define RXbuffer6_size     30
#define RXbuffer4_size     12

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


typedef struct 
{
	uint8_t Boss_State;      //0x01:启动  0x00:停止  控制小车到达任务区后做完任务后是否使能速度
	uint8_t Boss_State_Last;
	uint8_t WC_PIT_R;        //接收到的坑位
	uint16_t X_data;
	
}U4_R_data;



void UART_Task_Init(void);
void Uart3_task(void);
void Uart4_task(void);
void Uart6_task(void);
void Uart2_task(void);
void Control_Emm_Angle(void);
void Control_Emm_Angle_2(void);
void Usart4_send(void);       						 //某一刻只执行一次
void Car_task2(void);
void Usart3_send_HWT101_Zero(void);



#endif
