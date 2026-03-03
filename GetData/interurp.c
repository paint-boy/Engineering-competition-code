#include "interurp.h"

int16_t YawL = 0;
int16_t YawH = 0;
int VL = 0;
int VH = 0;

int16_t RWzL = 0;
int16_t RWzH = 0;
int16_t WzL = 0;
int16_t WzH = 0;

uint8_t Rxbuffer1[RXbuffer1_size] = {0};
uint8_t Rxbuffer2[RXbuffer2_size] = {0};
uint8_t Rxbuffer3[RXbuffer3_size] = {0};
uint8_t Rxbuffer6[RXbuffer6_size] = {0};
uint8_t Rxbuffer4[RXbuffer4_size] = {0};

uint8_t Rxbuffer1_2[RXbuffer1_size] = {0};
uint8_t Rxbuffer2_2[RXbuffer2_size] = {0};
uint8_t Rxbuffer3_2[RXbuffer3_size] = {0};
uint8_t Rxbuffer6_2[RXbuffer6_size] = {0};
uint8_t Rxbuffer4_2[RXbuffer4_size] = {0};

U4_R_data U4_R_Data;
extern Parameter_TypeDef_Car Car_data;
extern uint8_t test_flag;

PID_TypeDef Emm_pid;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4)      //10ms定时器
	{
		Car_Control();
	}
	else if(htim->Instance == TIM5)
	{
		Emm_Pos_Control(Car_data.Motor_Angle);
	}
}
uint8_t Usart4_send_Buffer[3] = {0};
uint8_t success_flag = 0;
void Usart4_send()         						 //某一刻只执行一次
{
	//与上部分主控通信数据处理
	Usart4_send_Buffer[0] = 0x55;
	Usart4_send_Buffer[1] = 0x01;
	Usart4_send_Buffer[2] = 0x45;
	HAL_UART_Transmit_DMA(&huart4, Usart4_send_Buffer, sizeof(Usart4_send_Buffer));
}
uint8_t HWT101_Zero_Buffer1[5] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
uint8_t HWT101_Zero_Buffer2[5] = {0xFF, 0xAA, 0x48, 0x01, 0x00};
uint8_t HWT101_Zero_Buffer3[5] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
uint8_t HWT101_Zero_Buffer4[5] = {0xFF, 0xAA, 0x00, 0x00, 0x00};
void Usart3_send_HWT101_Zero()
{
	//自动获取零偏校正陀螺仪，需要等待20秒
	HAL_UART_Transmit_DMA(&huart3, HWT101_Zero_Buffer1, sizeof(HWT101_Zero_Buffer1));//解锁
	HAL_Delay(1000);
	HAL_UART_Transmit_DMA(&huart3, HWT101_Zero_Buffer2, sizeof(HWT101_Zero_Buffer2));//自动获取零偏
	HAL_Delay(30000);	
	HAL_UART_Transmit_DMA(&huart3, HWT101_Zero_Buffer3, sizeof(HWT101_Zero_Buffer3));//解锁
	HAL_Delay(1000);
	HAL_UART_Transmit_DMA(&huart3, HWT101_Zero_Buffer4, sizeof(HWT101_Zero_Buffer4));//保存
}

void Uart4_task()
{
	static uint8_t High_bite = 0;
	static uint8_t Low_bite = 0;
		for (uint8_t i = 0; i < 8; i++)
		{
			if(Rxbuffer4[i] == 0x55 && Rxbuffer4[(i+5)%6] == 0x45)
			{
				if(Rxbuffer4[(i+1)%6] == 0x01)
				{
					U4_R_Data.Boss_State = 0x01;   //启动
				}
				else if(Rxbuffer4[(i+1)%6] == 0x00)
				{
					U4_R_Data.Boss_State = 0x00;   //停止
				}
				U4_R_Data.WC_PIT_R = Rxbuffer4[(i+2)%6];      //接收坑位
				High_bite = Rxbuffer4[(i+3)%6];
				Low_bite = Rxbuffer4[(i+4)%6];
				U4_R_Data.X_data = (High_bite<<8)|Low_bite;

				break;
			}
		}
		U4_R_Data.Boss_State_Last = U4_R_Data.Boss_State;
}

uint8_t Car_Err_flag = 0;
void Control_Emm_Angle()         //坑位处理后得到转盘的旋转角度
{
	if(U4_R_Data.X_data > 370)    //说明球在右边
	{
		switch(U4_R_Data.WC_PIT_R)
		{
			case 1: Car_data.Motor_Angle = 90 + 30;
						break;
			case 2: Car_data.Motor_Angle = 85;
						break;
			case 3: Car_data.Motor_Angle = 90 - 37;
						break;
		}
		Car_Err_flag = 1;
	}
	else if(U4_R_Data.X_data < 270)
	{
		switch(U4_R_Data.WC_PIT_R)
		{
			case 1: Car_data.Motor_Angle = 90 + 40;
						break;
			case 2: Car_data.Motor_Angle = 95;
						break;
			case 3: Car_data.Motor_Angle = 90 - 30;
						break;
		}
		Car_Err_flag = 2;
	}
	else
	{
		switch(U4_R_Data.WC_PIT_R)
		{
			case 1: Car_data.Motor_Angle = 90 + 33;
						break;
			case 2: Car_data.Motor_Angle = 91;
						break;
			case 3: Car_data.Motor_Angle = 90 - 33;
						break;
		}
		test_flag = 1;
	}
}
void Control_Emm_Angle_2() 
{
	if(Car_Err_flag == 1)    //说明球在右边
	{
		switch(U4_R_Data.WC_PIT_R)
		{
			case 0: break;
			
			case 1: Car_data.Motor_Angle = 90 + 33;
						break;
			case 2: Car_data.Motor_Angle = 90;
						break;
			case 3: Car_data.Motor_Angle = 90 - 34;
						break;
		}
	}
	else if(Car_Err_flag == 2)
	{
		switch(U4_R_Data.WC_PIT_R)
		{
			case 0: break;
			
			case 1: Car_data.Motor_Angle = 90 + 34;
						break;
			case 2: Car_data.Motor_Angle = 90;
						break;
			case 3: Car_data.Motor_Angle = 90 - 33;
						break;
		}
	}
	else
	{
		switch(U4_R_Data.WC_PIT_R)
		{
			case 0: break;
			
			case 1: Car_data.Motor_Angle = 90 + 34;
						break;
			case 2: Car_data.Motor_Angle = 90;
						break;
			case 3: Car_data.Motor_Angle = 90 - 34;
						break;
		}
	}
}
void UART_Task_Init(void)
{ 
	  HAL_UART_Receive_DMA(&huart2, Rxbuffer2, RXbuffer2_size);
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE); 
	  HAL_UART_Receive_DMA(&huart3, Rxbuffer3, RXbuffer3_size);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE); 
	  HAL_UART_Receive_DMA(&huart6, Rxbuffer6, RXbuffer6_size);
	__HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE); 
		HAL_UART_Receive_DMA(&huart4, Rxbuffer4, RXbuffer4_size);
	__HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
}
void Uart3_task()
{
		/*检索并解包hwt101*/
		for(uint8_t i = 0; i<22; i++)
		{
			if(Rxbuffer3[i] == 0x55 && Rxbuffer3[(i+1)%22] == 0x53)
			{
				YawL = Rxbuffer3[(i + 6)%22];
				YawH = Rxbuffer3[(i + 7)%22];
				VL = Rxbuffer3[(i + 8)%22];
				VH = Rxbuffer3[(i + 9)%22];
				Yaw_struct.Yaw = (float)((YawH<<8)|YawL)/32768*180;
				if(Yaw_struct.Yaw > 180)
				{
					Yaw_struct.Yaw = -(360.0f - Yaw_struct.Yaw);
				}
				break;
			}
		}
}
void Uart2_task()
{
		//调试数据处理
}
void Uart6_task()
{
		//步进电机数据处理
}

