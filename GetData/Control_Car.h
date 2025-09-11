#ifndef _CONTROL_CAR_H_
#define _CONTROL_CAR_H_

//welcome to my github
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "PID.h"
#include "math.h"
#include "usart.h"
#include "stdio.h"
#include "Encode.h"
#include "Emm_V5.h"

#define Diameter  15.0f
#define M_PI_F 3.141592653589793f

//定义小车速度
#define	Low_speed     1.0f
#define	Middle_speed  2.0f
#define High_speed    3.0f
#define Error         2.3f

typedef struct 
{
	PID_TypeDef Yaw_pid;
	float Yaw;
	float Tar_Yaw;
	float Yaw_Gz;
	
}Yaw_TypeDef;

typedef struct 
{
	float Line_speed;
  uint8_t flag_L_R;
	
	float Car_Pos;
	float Motor_L_Pos;
	float Motor_R_Pos;
	float Motor_L_Out;
	float Motor_R_Out;
	
}Parameter_TypeDef_Car;

//张大头电机步距角1.8度除于256得到0.00703125（单位是角度/脉冲）
enum Emm_Angle
{
	Angle_45       = 6400,
	Angle_90       = 12800,
	Angle_135      = 19200,
	Angle_180      = 25600,
	Angle_225      = 32000,
	Angle_270      = 38400,
	Angle_315      = 44800,
	Angle_360      = 51200,
};

void Yaw_PID_Control(void);
void Yaw_pid_Init(void);
void Sport_Car(void);
void Car_Control(void);
void ResetHWT101(void);
void Car_turn_corner(float Yaw_Current, float Pos_Current);
void Car_Drive_route(void);
void Emm_Pos_Control(uint32_t clk);


extern Yaw_TypeDef Yaw_struct;



#endif
