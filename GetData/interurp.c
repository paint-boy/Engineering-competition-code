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


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4)      //10ms定时器
	{
		Car_Control();
		
	}

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance == USART2)      //调试
	{
   		HAL_UARTEx_ReceiveToIdle_DMA(&huart2 , (uint8_t *)Rxbuffer2 , sizeof(Rxbuffer2));
	}
	else if(huart->Instance == USART6)   //张大头步进电机
	{
	
		HAL_UARTEx_ReceiveToIdle_DMA(&huart6 , (uint8_t *)Rxbuffer6 , sizeof(Rxbuffer6));
	}
	else if(huart->Instance == UART4)   //与上面主控通信
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
		HAL_UARTEx_ReceiveToIdle_DMA(&huart4 , (uint8_t *)Rxbuffer4 , sizeof(Rxbuffer4));
	}
	else if(huart->Instance == USART3) //hwt101
	{
		/*检索并解包hwt101*/
		for(uint8_t i = 0; i<22; i++)
		{
			if(Rxbuffer3[i] == 0x55 && Rxbuffer3[(i+1)%22] == 0x52)
			{
				RWzL = Rxbuffer3[(i + 4)%22];
				RWzH = Rxbuffer3[(i + 5)%22];
				WzL = Rxbuffer3[(i + 6)%22];
				WzH = Rxbuffer3[(i + 7)%22];
				
				Yaw_struct.Yaw_Gz = (float)((WzH<<8)|WzL)/32768.0f*2000.0f;
				break;
			}
		}
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
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3 , (uint8_t *)Rxbuffer3 , sizeof(Rxbuffer3));
	}
}





