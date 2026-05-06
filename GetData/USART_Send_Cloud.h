#ifndef __USART_SEND_CLOUD_H__
#define __USART_SEND_CLOUD_H__

#include "main.h"
#include "stdint.h"

#define USART_SEND_CLOUD_TX_BUFFER_SIZE 64U

void USART_Send_Cloud_RequestAll(void);
void USART_Send_Cloud_RequestUpdated(void);
HAL_StatusTypeDef USART_Send_Cloud_Task(void);

HAL_StatusTypeDef USART_Send_Cloud_SendSlider(uint8_t target, float value);
HAL_StatusTypeDef USART_Send_Cloud_SendRool(void);
HAL_StatusTypeDef USART_Send_Cloud_SendPitch(void);
HAL_StatusTypeDef USART_Send_Cloud_SendNDUN(void);

#endif
