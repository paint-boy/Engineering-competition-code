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


uint8_t Boss_State = 0;   //0x01:启动  0x00:停止  上主控发送来的数据
uint8_t Boss_State_Last = 0;   


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4)      //10ms定时器
	{
		Car_Control();
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

void Uart2_task()
{
		//调试数据处理
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
void Uart4_task()
{
		for (uint8_t i = 0; i < 3; i++)
		{
			if(Rxbuffer4[i] == 0x55 && Rxbuffer4[(i+2)%6] == 0x45)
			{
				if(Rxbuffer4[(i+1)%6] == 0x01)
				{
					Boss_State = 0x01;   //启动
				}
				else if(Rxbuffer4[(i+1)%6] == 0x00)
				{
					Boss_State = 0x00;   //停止
				}
				break;
			}
		}
		Boss_State_Last = Boss_State;
}
void Uart6_task()
{
		//步进电机数据处理
}


