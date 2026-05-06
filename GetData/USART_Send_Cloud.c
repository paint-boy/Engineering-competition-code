#include "USART_Send_Cloud.h"

#include "Bule_Tooch.h"
#include "usart.h"

#define USART_SEND_CLOUD_PENDING_ROOL  0x01U
#define USART_SEND_CLOUD_PENDING_PITCH 0x02U
#define USART_SEND_CLOUD_PENDING_NDUN  0x04U

static uint8_t USART_Send_Cloud_TxBuffer[USART_SEND_CLOUD_TX_BUFFER_SIZE] = {0};
static uint8_t USART_Send_Cloud_PendingMask = 0;
static uint8_t USART_Send_Cloud_NextIndex = 0;

static uint8_t USART_Send_Cloud_IsTargetValid(uint8_t target)
{
	return (target == 'R') || (target == 'P') || (target == 'N');
}

static uint8_t USART_Send_Cloud_AppendByte(uint8_t *buffer, uint16_t *index, uint16_t size, uint8_t data)
{
	if (*index >= size)
	{
		return 0;
	}

	buffer[*index] = data;
	(*index)++;
	return 1;
}

static uint8_t USART_Send_Cloud_AppendUnsigned(uint8_t *buffer, uint16_t *index, uint16_t size, uint32_t value)
{
	uint8_t temp[10] = {0};
	uint8_t count = 0;

	do
	{
		temp[count] = (uint8_t)('0' + (value % 10U));
		value /= 10U;
		count++;
	} while ((value != 0U) && (count < sizeof(temp)));

	while (count > 0U)
	{
		count--;
		if (USART_Send_Cloud_AppendByte(buffer, index, size, temp[count]) == 0U)
		{
			return 0;
		}
	}

	return 1;
}

static uint8_t USART_Send_Cloud_AppendFloat2(uint8_t *buffer, uint16_t *index, uint16_t size, float value)
{
	uint8_t negative = 0;
	uint32_t scaled = 0;
	uint32_t integer = 0;
	uint32_t fraction = 0;

	if (value < 0.0f)
	{
		negative = 1;
		value = -value;
	}

	scaled = (uint32_t)((value * 100.0f) + 0.5f);
	integer = scaled / 100U;
	fraction = scaled % 100U;

	if (negative != 0U)
	{
		if (USART_Send_Cloud_AppendByte(buffer, index, size, (uint8_t)'-') == 0U)
		{
			return 0;
		}
	}

	if (USART_Send_Cloud_AppendUnsigned(buffer, index, size, integer) == 0U)
	{
		return 0;
	}

	if (USART_Send_Cloud_AppendByte(buffer, index, size, (uint8_t)'.') == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendByte(buffer, index, size, (uint8_t)('0' + (fraction / 10U))) == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendByte(buffer, index, size, (uint8_t)('0' + (fraction % 10U))) == 0U)
	{
		return 0;
	}

	return 1;
}

static uint16_t USART_Send_Cloud_BuildSliderFrame(uint8_t target, float value, uint8_t *buffer, uint16_t size)
{
	uint16_t index = 0;

	if ((buffer == 0) || (USART_Send_Cloud_IsTargetValid(target) == 0U))
	{
		return 0;
	}

	if (USART_Send_Cloud_AppendByte(buffer, &index, size, (uint8_t)'[') == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendByte(buffer, &index, size, (uint8_t)'s') == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendByte(buffer, &index, size, (uint8_t)',') == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendByte(buffer, &index, size, target) == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendByte(buffer, &index, size, (uint8_t)',') == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendFloat2(buffer, &index, size, value) == 0U)
	{
		return 0;
	}
	if (USART_Send_Cloud_AppendByte(buffer, &index, size, (uint8_t)']') == 0U)
	{
		return 0;
	}

	return index;
}

static uint8_t USART_Send_Cloud_GetPendingByIndex(uint8_t index, uint8_t *target, float *value, uint8_t *mask)
{
	if (index == 0U)
	{
		*target = (uint8_t)'R';
		*value = Bule_tooch_Data.Rool_angle;
		*mask = USART_SEND_CLOUD_PENDING_ROOL;
	}
	else if (index == 1U)
	{
		*target = (uint8_t)'P';
		*value = Bule_tooch_Data.Pitch_angle;
		*mask = USART_SEND_CLOUD_PENDING_PITCH;
	}
	else
	{
		*target = (uint8_t)'N';
		*value = Bule_tooch_Data.NDUN;
		*mask = USART_SEND_CLOUD_PENDING_NDUN;
	}

	return (USART_Send_Cloud_PendingMask & *mask) != 0U;
}

void USART_Send_Cloud_RequestAll(void)
{
	USART_Send_Cloud_PendingMask |= USART_SEND_CLOUD_PENDING_ROOL;
	USART_Send_Cloud_PendingMask |= USART_SEND_CLOUD_PENDING_PITCH;
	USART_Send_Cloud_PendingMask |= USART_SEND_CLOUD_PENDING_NDUN;
}

void USART_Send_Cloud_RequestUpdated(void)
{
	if (Bule_tooch_Data.Slider_Target == 'R')
	{
		USART_Send_Cloud_PendingMask |= USART_SEND_CLOUD_PENDING_ROOL;
	}
	else if (Bule_tooch_Data.Slider_Target == 'P')
	{
		USART_Send_Cloud_PendingMask |= USART_SEND_CLOUD_PENDING_PITCH;
	}
	else if (Bule_tooch_Data.Slider_Target == 'N')
	{
		USART_Send_Cloud_PendingMask |= USART_SEND_CLOUD_PENDING_NDUN;
	}
}

HAL_StatusTypeDef USART_Send_Cloud_SendSlider(uint8_t target, float value)
{
	uint16_t length = 0;

	if (huart4.gState != HAL_UART_STATE_READY)
	{
		return HAL_BUSY;
	}

	length = USART_Send_Cloud_BuildSliderFrame(target,
											  value,
											  USART_Send_Cloud_TxBuffer,
											  USART_SEND_CLOUD_TX_BUFFER_SIZE);
	if (length == 0U)
	{
		return HAL_ERROR;
	}

	return HAL_UART_Transmit_DMA(&huart4, USART_Send_Cloud_TxBuffer, length);
}

HAL_StatusTypeDef USART_Send_Cloud_SendRool(void)
{
	return USART_Send_Cloud_SendSlider((uint8_t)'R', Bule_tooch_Data.Rool_angle);
}

HAL_StatusTypeDef USART_Send_Cloud_SendPitch(void)
{
	return USART_Send_Cloud_SendSlider((uint8_t)'P', Bule_tooch_Data.Pitch_angle);
}

HAL_StatusTypeDef USART_Send_Cloud_SendNDUN(void)
{
	return USART_Send_Cloud_SendSlider((uint8_t)'N', Bule_tooch_Data.NDUN);
}

HAL_StatusTypeDef USART_Send_Cloud_Task(void)
{
	uint8_t i = 0;
	uint8_t target = 0;
	float value = 0.0f;
	uint8_t mask = 0;
	HAL_StatusTypeDef status = HAL_ERROR;

	if (USART_Send_Cloud_PendingMask == 0U)
	{
		return HAL_OK;
	}

	if (huart4.gState != HAL_UART_STATE_READY)
	{
		return HAL_BUSY;
	}

	for (i = 0; i < 3U; i++)
	{
		if (USART_Send_Cloud_GetPendingByIndex(USART_Send_Cloud_NextIndex, &target, &value, &mask) != 0U)
		{
			status = USART_Send_Cloud_SendSlider(target, value);
			if (status == HAL_OK)
			{
				USART_Send_Cloud_PendingMask &= (uint8_t)(~mask);
				USART_Send_Cloud_NextIndex++;
				if (USART_Send_Cloud_NextIndex >= 3U)
				{
					USART_Send_Cloud_NextIndex = 0;
				}
			}
			return status;
		}

		USART_Send_Cloud_NextIndex++;
		if (USART_Send_Cloud_NextIndex >= 3U)
		{
			USART_Send_Cloud_NextIndex = 0;
		}
	}

	return HAL_OK;
}
