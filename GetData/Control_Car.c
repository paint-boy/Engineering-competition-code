#include "Control_Car.h"

extern U4_R_data U4_R_Data;
Parameter_TypeDef_Car Car_data;

Yaw_TypeDef Yaw_struct;

void Yaw_pid_Init()
{
	Yaw_struct.Yaw_pid.Kp                  = 0.15f; 
	Yaw_struct.Yaw_pid.Ki                  = 0.0f;
	Yaw_struct.Yaw_pid.Kd                  = 0.0f;
	Yaw_struct.Yaw_pid.I_Max               = 0.0f;
	Yaw_struct.Yaw_pid.Out_Max             = 0.8f;
	Yaw_struct.Yaw_pid.Out                 = 0.0f;
}

//航向环闭环
void Yaw_PID_Control()
{
	Yaw_struct.Yaw_pid.Current = Yaw_struct.Yaw;
	Position_PID(&Yaw_struct.Yaw_pid, Yaw_struct.Tar_Yaw);
//	Yaw_struct.Yaw_pid.Out
}

//小车运动
void Sport_Car()
{
	//直线速度
	float Speed = (motors[Motor_0].Parameter.Speed + motors[Motor_1].Parameter.Speed)/2.0f;
	motors[Motor_0].Parameter.Tar_Speed = speed_control(motors[Motor_0].Parameter.Acc, Speed, -Car_data.Line_speed, motors[Motor_0].Parameter.dt);
	motors[Motor_1].Parameter.Tar_Speed = speed_control(motors[Motor_1].Parameter.Acc, Speed, -Car_data.Line_speed, motors[Motor_1].Parameter.dt);
	
	Gat_Motor_data(Motor_0);
	Gat_Motor_data(Motor_1);
	
	//获取电机路程
	Car_data.Motor_L_Pos = -motors[Motor_0].Parameter.Pos;
	Car_data.Motor_R_Pos = -motors[Motor_1].Parameter.Pos;
	Car_data.Car_Pos = (Car_data.Motor_L_Pos + Car_data.Motor_R_Pos)/2.0f *15.0f;
	
	//加入航向环控制
	Yaw_PID_Control();
	//闭环控制
	motors[Motor_0].speed_pid.Current = motors[Motor_0].Parameter.Speed;
	motors[Motor_1].speed_pid.Current = motors[Motor_1].Parameter.Speed;
	
	Position_PID(&motors[Motor_1].speed_pid , motors[Motor_1].Parameter.Tar_Speed + Yaw_struct.Yaw_pid.Out);
	Position_PID(&motors[Motor_0].speed_pid , motors[Motor_0].Parameter.Tar_Speed - Yaw_struct.Yaw_pid.Out);
	
	Car_data.Motor_L_Out = motors[Motor_1].speed_pid.Out;
	Car_data.Motor_R_Out = motors[Motor_0].speed_pid.Out;
	
	//赋予电机
	uint32_t CCR1 = (uint32_t)fabsf(Car_data.Motor_L_Out);
	uint32_t CCR2 = (uint32_t)fabsf(Car_data.Motor_R_Out);
	
	if (CCR1 > PWM_ARR) CCR1 = PWM_ARR;
	if (CCR2 > PWM_ARR) CCR2 = PWM_ARR;
	
	if(Car_data.Motor_R_Out > 0)
	{
		HAL_GPIO_WritePin(motors[Motor_0].Motor_Dir_GPIO_Port , motors[Motor_0].Motor_Dir_Pin , 0);
	}
	else
	{
		HAL_GPIO_WritePin(motors[Motor_0].Motor_Dir_GPIO_Port , motors[Motor_0].Motor_Dir_Pin , 1);
	}
	
	if(Car_data.Motor_L_Out > 0)
	{
		HAL_GPIO_WritePin(motors[Motor_1].Motor_Dir_GPIO_Port , motors[Motor_1].Motor_Dir_Pin , 0);
	}
	else
	{
		HAL_GPIO_WritePin(motors[Motor_1].Motor_Dir_GPIO_Port , motors[Motor_1].Motor_Dir_Pin , 1);
	}
	
	*motors[Motor_1].ccr = CCR1;
	*motors[Motor_0].ccr = CCR2;
	
//	printf("%d,%d\n",motors[Motor_1].tim -> CNT,motors[Motor_1].cnt);
	
	
}
//坦克主控函数
void Car_Control()
{
	Car_Drive_route();
	Car_turn_corner(Yaw_struct.Yaw, Car_data.Car_Pos);
	Sport_Car();
}
uint8_t CW_OR_CCW = 1;
//坦克行驶路径
void Car_Drive_route()
{
	static uint8_t flag = 0;
	static float Pos_temp = 0.0f;
	if(flag == 0)
	{
		Car_data.Line_speed = Middle_speed;
		if(Car_data.Car_Pos > 43.0f)
		{
			Car_data.flag_L_R = 1;        //第一处转弯
			flag = 1;
		}
	}
	else if(flag == 1)
	{
		if(Car_data.flag_L_R == 0)
		{
			Pos_temp = Car_data.Car_Pos;
			flag = 2;
		}
	}
	else if(flag == 2)
	{
		if(Car_data.Car_Pos >= Pos_temp + 12.0f)
		{
			Car_data.flag_L_R = 2;        //第二处转弯
			flag = 3;
		}
	}
	else if(flag == 3)
	{
		if(Car_data.flag_L_R == 0)
		{
			Pos_temp = Car_data.Car_Pos;
			flag = 4;
		}
	}
	else if(flag == 4)
	{
		if(Car_data.Car_Pos >= Pos_temp + 46.0f)
		{
			Car_data.flag_L_R = 1;        //第三处转弯
			flag = 5;
		}
	}
	else if(flag == 5)
	{
		if(Car_data.flag_L_R == 0)
		{
			Pos_temp = Car_data.Car_Pos;
			flag = 6;
		}
	}
	else if(flag == 6)
	{
		if(Car_data.Car_Pos >= Pos_temp + 31.0f)
		{
			Car_data.flag_L_R = 2;        //第四处转弯
			flag = 7;
		}
	}
	else if(flag == 7)
	{
		if(Car_data.flag_L_R == 0)
		{
			Pos_temp = Car_data.Car_Pos;
			flag = 8;
		}
	}
	else if(flag == 8)
	{
		if(Car_data.Car_Pos >= Pos_temp + 47.5f)
		{
			Car_data.flag_L_R = 2;        //第五处转弯
			flag = 9;
		}
	}
	else if(flag == 9)
	{
		if(Car_data.flag_L_R == 0)        
		{
			Car_data.Line_speed = Low_speed;
			Usart4_send();     					 	//转弯成功
			Car_data.Motor_Angle = 90.0f;	//转盘旋转90度
			Pos_temp = Car_data.Car_Pos;
			flag = 10;
		}
	}
	else if(flag == 10)
	{
		if(Car_data.Car_Pos >= Pos_temp + 44.5f)
		{
			Yaw_struct.Tar_Yaw = -90.3f;
			Car_data.Line_speed = 0;                 //排爆区
			Usart4_send();     					 					  	//停车成功
			if(U4_R_Data.WC_PIT_R != 0)
			{
				Control_Emm_Angle();
				flag = 100;
			}
		}
	}
	else if(flag == 100)
	{
		if(U4_R_Data.Boss_State == 0x01)          
		{
			if(U4_R_Data.WC_PIT_R == 3)
			{
				CW_OR_CCW = 0;
				Car_data.Motor_Angle = 90;
			}
			else
			{
				CW_OR_CCW = 1;
				Car_data.Motor_Angle = 270;
			}
			static uint16_t i = 0;
			i++;
			if(i > 300)
			{
				U4_R_Data.Boss_State = 0;
				Usart4_send();      //270度转向成功
				flag = 11;
			}
		}
	}
	else if(flag == 11)
	{
		//排爆区等待
		if(U4_R_Data.Boss_State == 0x01)           //启动小车
		{
			U4_R_Data.Boss_State = 0;
			Pos_temp = Car_data.Car_Pos;
			Car_data.Line_speed = Middle_speed;                 
			flag = 12;
		}
	}
	else if(flag == 12)
	{
		if(Car_data.Car_Pos >= Pos_temp + 175.0f)
		{
			Car_data.Line_speed = 0;                 //打靶区
			flag = 13;
		}
	}
	else if(flag == 13)
	{
		//打靶区等待
		if(U4_R_Data.Boss_State  == 0x01 && U4_R_Data.Boss_State_Last == 0x00)           //停止小车
		{
			Pos_temp = Car_data.Car_Pos;
			Car_data.Line_speed = Middle_speed;                 
			flag = 14;
		}
	}
	else if(flag == 14)
	{
		if(Car_data.Car_Pos >= Pos_temp + 37.5f)
		{
			Car_data.flag_L_R = 2;        //第六处转弯
			flag = 15;
		}
	}
	else if(flag == 15)
	{
		if(Car_data.flag_L_R == 0)
		{
			Pos_temp = Car_data.Car_Pos;
			flag = 16;
		}
	}
	else if(flag == 16)
	{
		if(Car_data.Car_Pos >= Pos_temp + 60.0f)
		{
			Car_data.Line_speed = 0;                 //人质区
			flag = 17;
		}
	}
	else if(flag == 17)
	{
		//人质区等待
		if(U4_R_Data.Boss_State == 0x01 && U4_R_Data.Boss_State_Last == 0x00)           //启动小车
		{
			Pos_temp = Car_data.Car_Pos;
			Car_data.Line_speed = Middle_speed;                 
			flag = 18;
		}
/* 		static uint16_t i = 0;
		i++;
		if(i > 500)
		{
			Yaw_struct.Tar_Yaw = -179.8f;
			Pos_temp = Car_data.Car_Pos;
			Car_data.Line_speed = Middle_speed;                 
			flag = 18;
		}
 */	}
	else if(flag == 18)
	{
		if(Car_data.Car_Pos >= Pos_temp + 170.0f)
		{
			Car_data.Line_speed = 0;                //终点
			flag = 19;
		}
	}

}

//坦克转弯函数，选择左右转，转向角度为90度
void Car_turn_corner(float Yaw_Current, float Pos_Current)
{
    static uint8_t flag = 0;
    static float Current_Temp = 0.0f;
    static float Yaw_Temp = 0.0f;
    static uint8_t last_flag_L_R = 0; // 新增

    // 只在flag_L_R从0变为非0时进入
    if(flag == 0 && Car_data.flag_L_R != 0 && last_flag_L_R == 0)
    {
        Current_Temp = Pos_Current;
        Yaw_Temp = Yaw_struct.Tar_Yaw;
        flag = 1;
    }
    else if(flag == 1)
    {
        if(Car_data.flag_L_R == 1)
        {
            Yaw_struct.Tar_Yaw = Yaw_Temp + (((Pos_Current - Current_Temp)/(Diameter*M_PI_F))*90.0f);
        }
        else if(Car_data.flag_L_R == 2)
        {
            Yaw_struct.Tar_Yaw = Yaw_Temp + (((Pos_Current - Current_Temp)/(Diameter*M_PI_F))*-90.0f);
        }
        if(Pos_Current - Current_Temp >= Diameter*M_PI_F || Pos_Current - Current_Temp <= -Diameter*M_PI_F)
        {
            flag = 2;
        }
    }
    else if(flag == 2)
    {
        flag = 0;
        Car_data.flag_L_R = 0;
    }
    last_flag_L_R = Car_data.flag_L_R; // 记录本次
}


//HWT101角度置零
void ResetHWT101(void)
{
	uint8_t temp[]={0xff,0xAA,0X76,0x00,0x00};
	HAL_UART_Transmit_DMA(&huart3, temp, 5);
}

void Emm_Pos_Control(float Angle)
{
	static uint32_t Angle_Tick = 0;
	Angle_Tick = (uint32_t)(1208.888 * Angle);
	Emm_V5_Pos_Control(1,CW_OR_CCW,180,0,Angle_Tick,true,false);
}












