#include "interurp.h"

int16_t YawL = 0;
int16_t YawH = 0;
int VL = 0;
int VH = 0;

int16_t RWzL = 0;
int16_t RWzH = 0;
int16_t WzL = 0;
int16_t WzH = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4)      //10ms定时器
	{
		Car_Control();
		
	}

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance == USART2)
	{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2 , (uint8_t *)rxbuffer , sizeof(rxbuffer));
	}
	else if(huart->Instance == USART6)
	{
	
		
	}
	else if(huart->Instance == UART4)
	{
	
		
	}
	else if(huart->Instance == USART3)
	{
		/*检索并解包hwt101*/
		for(uint8_t i = 0; i<22; i++)
		{
			if(rxbuffer2[i] == 0x55 && rxbuffer2[(i+1)%22] == 0x52)
			{
				RWzL = rxbuffer2[(i + 4)%22];
				RWzH = rxbuffer2[(i + 5)%22];
				WzL = rxbuffer2[(i + 6)%22];
				WzH = rxbuffer2[(i + 7)%22];
				
				Yaw_struct.Yaw_Gz = (float)((WzH<<8)|WzL)/32768.0f*2000.0f;
				break;
			}
		}
		for(uint8_t i = 0; i<22; i++)
		{
			if(rxbuffer2[i] == 0x55 && rxbuffer2[(i+1)%22] == 0x53)
			{
				YawL = rxbuffer2[(i + 6)%22];
				YawH = rxbuffer2[(i + 7)%22];
				VL = rxbuffer2[(i + 8)%22];
				VH = rxbuffer2[(i + 9)%22];
				Yaw_struct.Yaw = (float)((YawH<<8)|YawL)/32768*180;
				if(Yaw_struct.Yaw > 180)
				{
					Yaw_struct.Yaw = -(360.0f - Yaw_struct.Yaw);
				}
				break;
			}
		}
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3 , (uint8_t *)rxbuffer2 , sizeof(rxbuffer2));
	}
}





