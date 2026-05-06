#include "Control_Car.h"

extern U4_R_data U4_R_Data;

Parameter_TypeDef_Car Car_data;

Yaw_TypeDef Yaw_struct;
uint8_t test_flag = 0;
uint8_t lone_turn_flag = 0;
void Yaw_pid_Init()
{
	Yaw_struct.Yaw_pid.Kp = 0.15f;
	Yaw_struct.Yaw_pid.Ki = 0.0f;
	Yaw_struct.Yaw_pid.Kd = 0.0f;
	Yaw_struct.Yaw_pid.I_Max = 0.0f;
	Yaw_struct.Yaw_pid.Out_Max = 0.8f;
	Yaw_struct.Yaw_pid.Out = 0.0f;
}
// 坦克主控函数
void Car_Control()
{
	bule_car_key(&Bule_tooch_Data);

	// Car_Drive_route();
	// Car_turn_corner(Yaw_struct.Yaw, Car_data.Car_Pos);

	Sport_Car();
}
uint8_t CW_OR_CCW = 1;
float Pos_temp2 = 0.0f;

volatile float Diameter = 15.0f;
static uint8_t Task_flag = 0;
volatile static uint8_t Timer_Delay = 0;

// 坦克行驶路径
void Car_Drive_route()
{


	

}
// 坦克转弯函数，选择左右转，转向角度为90度
void Car_turn_corner(float Yaw_Current, float Pos_Current)
{
	static uint8_t flag = 0;
	static float Current_Temp = 0.0f;
	static float Yaw_Temp = 0.0f;
	static uint8_t last_flag_L_R = 0; // 新增

	// 只在flag_L_R从0变为非0时进入
	if (flag == 0 && Car_data.flag_L_R != 0 && last_flag_L_R == 0)
	{
		Current_Temp = Pos_Current;
		Yaw_Temp = Yaw_struct.Tar_Yaw;
		flag = 1;
	}
	else if (flag == 1)
	{
		if (Car_data.flag_L_R == 1)
		{
			Yaw_struct.Tar_Yaw = Yaw_Temp + (((Pos_Current - Current_Temp) / (Diameter * M_PI_F)) * 90.0f);
		}
		else if (Car_data.flag_L_R == 2)
		{
			Yaw_struct.Tar_Yaw = Yaw_Temp + (((Pos_Current - Current_Temp) / (Diameter * M_PI_F)) * -90.0f);
		}
		if (Pos_Current - Current_Temp >= Diameter * M_PI_F || Pos_Current - Current_Temp <= -Diameter * M_PI_F)
		{
			flag = 2;
		}
	}
	else if (flag == 2)
	{
		flag = 0;
		Car_data.flag_L_R = 0;
	}
	last_flag_L_R = Car_data.flag_L_R; // 记录本次
}
// HWT101角度置零
void ResetHWT101(void)
{
	uint8_t temp[] = {0xff, 0xAA, 0X76, 0x00, 0x00};
	HAL_UART_Transmit_DMA(&huart3, temp, 5);
}
void Emm_Pos_Control(float Angle)
{
	if(Angle > 0)
	{
		CW_OR_CCW = 0;
	}
	else
	{
		CW_OR_CCW = 1;
		Angle = fabsf(Angle);
	}
	static uint32_t Angle_Tick = 0;
	Angle_Tick = (uint32_t)(76.0f* Angle);
	Emm_V5_Pos_Control(2, CW_OR_CCW, 50, 0, Angle_Tick, true, false);
}

// 小车运动
void Sport_Car()
{
	// 直线速度
	float Speed = (motors[Motor_0].Parameter.Speed + motors[Motor_1].Parameter.Speed) / 2.0f;
	motors[Motor_0].Parameter.Tar_Speed = speed_control(motors[Motor_0].Parameter.Acc, Speed, -Car_data.Line_speed, motors[Motor_0].Parameter.dt);
	motors[Motor_1].Parameter.Tar_Speed = speed_control(motors[Motor_1].Parameter.Acc, Speed, -Car_data.Line_speed, motors[Motor_1].Parameter.dt);

	Gat_Motor_data(Motor_0);
	Gat_Motor_data(Motor_1);

	// 获取电机路程
	Car_data.Motor_L_Pos = -motors[Motor_0].Parameter.Pos;
	Car_data.Motor_R_Pos = -motors[Motor_1].Parameter.Pos;
	Car_data.Car_Pos = (Car_data.Motor_L_Pos + Car_data.Motor_R_Pos) / 2.0f * 15.0f;


	// 闭环控制
	motors[Motor_0].speed_pid.Current = motors[Motor_0].Parameter.Speed;
	motors[Motor_1].speed_pid.Current = motors[Motor_1].Parameter.Speed;
	if (lone_turn_flag == 1)
	{
		Position_PID(&motors[Motor_0].speed_pid, motors[Motor_0].Parameter.Tar_Speed - Yaw_struct.Yaw_pid.Out);
		Car_data.Motor_L_Out = 0.0f;
		Car_data.Motor_R_Out = motors[Motor_0].speed_pid.Out;
	}
	else if (lone_turn_flag == 2)
	{
		Position_PID(&motors[Motor_1].speed_pid, motors[Motor_1].Parameter.Tar_Speed + Yaw_struct.Yaw_pid.Out);
		Car_data.Motor_L_Out = motors[Motor_1].speed_pid.Out;
		Car_data.Motor_R_Out = 0.0f;
	}
	else
	{
		Position_PID(&motors[Motor_1].speed_pid, motors[Motor_1].Parameter.Tar_Speed + Yaw_struct.Yaw_pid.Out + Car_data.Turn_speed);
		Position_PID(&motors[Motor_0].speed_pid, motors[Motor_0].Parameter.Tar_Speed - Yaw_struct.Yaw_pid.Out - Car_data.Turn_speed);
		Car_data.Motor_L_Out = motors[Motor_1].speed_pid.Out;
		Car_data.Motor_R_Out = motors[Motor_0].speed_pid.Out;
	}
	// 赋予电机
	uint32_t CCR1 = (uint32_t)fabsf(Car_data.Motor_L_Out);
	uint32_t CCR2 = (uint32_t)fabsf(Car_data.Motor_R_Out);

	if (CCR1 > PWM_ARR)
		CCR1 = PWM_ARR;
	if (CCR2 > PWM_ARR)
		CCR2 = PWM_ARR;

	if (Car_data.Motor_R_Out > 0)
	{
		HAL_GPIO_WritePin(motors[Motor_0].Motor_Dir_GPIO_Port, motors[Motor_0].Motor_Dir_Pin, 0);
	}
	else
	{
		HAL_GPIO_WritePin(motors[Motor_0].Motor_Dir_GPIO_Port, motors[Motor_0].Motor_Dir_Pin, 1);
	}

	if (Car_data.Motor_L_Out > 0)
	{
		HAL_GPIO_WritePin(motors[Motor_1].Motor_Dir_GPIO_Port, motors[Motor_1].Motor_Dir_Pin, 0);
	}
	else
	{
		HAL_GPIO_WritePin(motors[Motor_1].Motor_Dir_GPIO_Port, motors[Motor_1].Motor_Dir_Pin, 1);
	}

	*motors[Motor_1].ccr = CCR1;
	*motors[Motor_0].ccr = CCR2;
}
// 航向环闭环
void Yaw_PID_Control()
{
	Yaw_struct.Yaw_pid.Current = Yaw_struct.Yaw;
	Position_PID(&Yaw_struct.Yaw_pid, Yaw_struct.Tar_Yaw);
}
