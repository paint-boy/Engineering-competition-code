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
#include "interurp.h"
#include "Bule_Car_Control.h"




#define M_PI_F 3.141592653589793f

//定义小车速度
#define	Low_speed     1.0f
#define	Middle_speed  2.0f
#define High_speed    3.0f
#define Error         2.3f

//各项参数
#define Turn_1     43.5f
#define Turn_2     27.0f
#define Turn_3     64.5f
#define Turn_4     43.0f
#define Turn_5     48.0f
#define Boom_Stop  47.0f
#define Turn6_Car_Err_flag_1  	       222.5f
#define Turn6_Car_Err_flag_2           218.5f
#define Turn6_Car_Err_flag__Not_1_2    220.5f
#define People_Help   								 79.0f
#define End_Stop    									 173.0f


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
	float Turn_speed;
  uint8_t flag_L_R;
	
	float Car_Pos;
	float Motor_L_Pos;
	float Motor_R_Pos;
	float Motor_L_Out;
	float Motor_R_Out;
	float Yaw_Angle;
	float Rool_Angle;
	float Pitch_Angle;
	float NDUN_Power;          

	float set_angle;


	
}Parameter_TypeDef_Car;


void Yaw_PID_Control(void);
void Yaw_pid_Init(void);
void Sport_Car(void);
void Car_Control(void);
void ResetHWT101(void);
void Car_turn_corner(float Yaw_Current, float Pos_Current);
void Car_Drive_route(void);
void Emm_Pos_Control(float Angle);

extern Parameter_TypeDef_Car Car_data;

extern Yaw_TypeDef Yaw_struct;
extern uint8_t Boss_State;
extern uint8_t Boss_State_Last;   




#endif
