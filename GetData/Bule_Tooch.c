#include "Bule_Tooch.h"
#include "string.h"
#include "usart.h"

#define BULE_TOOCH_MAX_FIELD_NUM 5
#define BULE_TOOCH_RX_FRAME_MAX BULE_TOOCH_DMA_BUFFER_SIZE

typedef struct
{
	const char *Start; /* 当前字段的起始地址 */
	const char *End;   /* 当前字段的结束地址，不包含 End 指向的字符 */
} Bule_Tooch_Token;

/* 解析后的蓝牙数据，全工程通过 Bule_Tooch.h 声明访问。 */
volatile Bule_tooch Bule_tooch_Data = {0};

/* USART2 DMA 专用接收缓冲区，长度必须大于最长协议包。 */
uint8_t Bule_Tooch_DmaBuffer[BULE_TOOCH_DMA_BUFFER_SIZE] = {0};

/* 非 DMA 字节接收模式下使用的临时拼包缓冲区。 */
static uint8_t Bule_Tooch_RxFrame[BULE_TOOCH_RX_FRAME_MAX] = {0};
static uint16_t Bule_Tooch_RxIndex = 0;
static uint8_t Bule_Tooch_RxBusy = 0;

/**
 * @brief 判断字符是否为空白字符。
 * @param ch 需要判断的字符。
 * @return 1 表示空格/制表符/回车/换行，0 表示不是空白字符。
 */
static uint8_t Bule_Tooch_IsSpace(char ch)
{
	return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n');
}

/**
 * @brief 判断字符是否为数字字符。
 * @param ch 需要判断的字符。
 * @return 1 表示 '0' 到 '9'，0 表示不是数字。
 */
static uint8_t Bule_Tooch_IsDigit(char ch)
{
	return (ch >= '0') && (ch <= '9');
}

/**
 * @brief 去掉字段首尾空白字符。
 * @param token 需要修剪的字段指针，函数会直接修改 token->Start 和 token->End。
 * @return 无。
 */
static void Bule_Tooch_TrimToken(Bule_Tooch_Token *token)
{
	while ((token->Start < token->End) && Bule_Tooch_IsSpace(*token->Start))
	{
		token->Start++;
	}

	while ((token->End > token->Start) && Bule_Tooch_IsSpace(*(token->End - 1)))
	{
		token->End--;
	}
}

/**
 * @brief 判断字段是否为指定的单个字符。
 * @param token 需要判断的字段。
 * @param ch 目标字符，例如 's'、'j'、'Y'。
 * @return 1 表示字段等于 ch，0 表示不相等。
 */
static uint8_t Bule_Tooch_TokenIsChar(const Bule_Tooch_Token *token, char ch)
{
	return ((token->End - token->Start) == 1) && (token->Start[0] == ch);
}

/**
 * @brief 判断字段是否等于指定字符串。
 * @param token 需要判断的字段。
 * @param str 目标字符串，例如 "open_bule"。
 * @return 1 表示字段内容等于 str，0 表示不相等。
 */
static uint8_t Bule_Tooch_TokenEqualsString(const Bule_Tooch_Token *token, const char *str)
{
	const char *p = token->Start;

	while ((p < token->End) && (*str != '\0'))
	{
		if (*p != *str)
		{
			return 0;
		}

		p++;
		str++;
	}

	return ((p == token->End) && (*str == '\0'));
}

/**
 * @brief 在缓冲区中查找一帧完整协议包的起止位置。
 * @param buffer 接收缓冲区首地址。
 * @param length 缓冲区有效长度。
 * @param frame_start 输出参数，找到的 '[' 下标。
 * @param frame_end 输出参数，找到的 ']' 下标。
 * @return 1 表示找到完整 [ ] 包，0 表示没有找到。
 */
static uint8_t Bule_Tooch_FindFrame(const uint8_t *buffer,
									uint16_t length,
									uint16_t *frame_start,
									uint16_t *frame_end)
{
	uint16_t i = 0;
	uint8_t start_found = 0;

	for (i = 0; i < length; i++)
	{
		if (buffer[i] == '[')
		{
			*frame_start = i;
			start_found = 1;
			break;
		}
	}

	if (start_found == 0)
	{
		return 0;
	}

	for (i = (uint16_t)(*frame_start + 1); i < length; i++)
	{
		if (buffer[i] == ']')
		{
			*frame_end = i;
			return 1;
		}
	}

	return 0;
}

/**
 * @brief 按逗号切分协议包内部字段。
 * @param frame 指向去掉 '[' 和 ']' 后的协议内容，例如 s,Y,190.33。
 * @param frame_len frame 的有效长度。
 * @param tokens 输出字段数组。
 * @param max_tokens tokens 数组最大容量。
 * @param token_count 输出字段数量。
 * @return 1 表示切分成功，0 表示字段为空或字段数量超过 max_tokens。
 */
static uint8_t Bule_Tooch_SplitTokens(const char *frame,
									  uint16_t frame_len,
									  Bule_Tooch_Token *tokens,
									  uint8_t max_tokens,
									  uint8_t *token_count)
{
	uint16_t i = 0;
	uint16_t token_start = 0;
	uint8_t count = 0;

	for (i = 0; i <= frame_len; i++)
	{
		if ((i == frame_len) || (frame[i] == ','))
		{
			if (count >= max_tokens)
			{
				return 0;
			}

			tokens[count].Start = &frame[token_start];
			tokens[count].End = &frame[i];
			Bule_Tooch_TrimToken(&tokens[count]);

			if (tokens[count].Start >= tokens[count].End)
			{
				return 0;
			}

			count++;
			token_start = (uint16_t)(i + 1);
		}
	}

	*token_count = count;
	return 1;
}

/**
 * @brief 将字段解析为 float。
 * @param token 需要解析的字段，例如 190.33、360、-1.5。
 * @param value 输出解析后的浮点数。
 * @return 1 表示解析成功，0 表示字段不是合法浮点数。
 * @note 支持正负号和小数点，不支持科学计数法。
 */
static uint8_t Bule_Tooch_ParseFloat(const Bule_Tooch_Token *token, float *value)
{
	const char *p = token->Start;
	float data = 0.0f;
	float decimal = 0.1f;
	uint8_t negative = 0;
	uint8_t has_digit = 0;

	if (p >= token->End)
	{
		return 0;
	}

	if ((*p == '-') || (*p == '+'))
	{
		negative = (uint8_t)(*p == '-');
		p++;
	}

	while ((p < token->End) && Bule_Tooch_IsDigit(*p))
	{
		data = (data * 10.0f) + (float)(*p - '0');
		has_digit = 1;
		p++;
	}

	if ((p < token->End) && (*p == '.'))
	{
		p++;
		while ((p < token->End) && Bule_Tooch_IsDigit(*p))
		{
			data += (float)(*p - '0') * decimal;
			decimal *= 0.1f;
			has_digit = 1;
			p++;
		}
	}

	if ((has_digit == 0) || (p != token->End))
	{
		return 0;
	}

	if (negative != 0)
	{
		data = -data;
	}

	*value = data;
	return 1;
}

/**
 * @brief 保存最近一次解析结果，并统计解析错误次数。
 * @param result 本次解析结果。
 * @return 原样返回 result，方便调用处直接 return。
 */
static Bule_Tooch_ParseResult Bule_Tooch_SaveResult(Bule_Tooch_ParseResult result)
{
	Bule_tooch_Data.Last_Result = result;

	if ((result != BULE_TOOCH_PARSE_OK) && (result != BULE_TOOCH_PARSE_NO_FRAME))
	{
		Bule_tooch_Data.Error_Count++;
	}

	return result;
}

/**
 * @brief 处理滑杆协议包。
 * @param tokens 已切分的字段数组，格式应为 s,Y/R/P/N,value。
 * @param token_count 字段数量，滑杆协议必须为 3。
 * @return Bule_Tooch_ParseResult，成功时会更新 Bule_tooch_Data 中对应角度或力的数据。
 */
static Bule_Tooch_ParseResult Bule_Tooch_UpdateSlider(const Bule_Tooch_Token *tokens,
													  uint8_t token_count)
{
	float value = 0.0f;
	char target = 0;

	if (token_count != 3)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_FORMAT);
	}

	if ((tokens[1].End - tokens[1].Start) != 1)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_TARGET);
	}

	target = tokens[1].Start[0];
	if ((target != 'Y') && (target != 'R') && (target != 'P') && (target != 'N'))
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_TARGET);
	}

	if (Bule_Tooch_ParseFloat(&tokens[2], &value) == 0)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_NUMBER);
	}

	Bule_tooch_Data.Data_Source = 's';
	Bule_tooch_Data.Slider_Target = (uint8_t)target;
	Bule_tooch_Data.Slider_Value = value;

	switch (target)
	{
	case 'Y':
		Bule_tooch_Data.Yaw_angle = value;
		break;
	case 'R':
		Bule_tooch_Data.Rool_angle = value;
		break;
	case 'P':
		Bule_tooch_Data.Pitch_angle = value;
		break;
	case 'N':
		Bule_tooch_Data.NDUN = value;
		break;
	default:
		break;
	}

	Bule_tooch_Data.Valid = 1;
	Bule_tooch_Data.Update_Flag = 1;
	Bule_tooch_Data.Frame_Count++;

	return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_OK);
}

/**
 * @brief 处理摇杆协议包。
 * @param tokens 已切分的字段数组，格式应为 j,value1,value2,value3,value4。
 * @param token_count 字段数量，摇杆协议必须为 5。
 * @return Bule_Tooch_ParseResult，成功时会更新 Joystick_Value[0..3] 和 Joystick_1..4。
 */
static Bule_Tooch_ParseResult Bule_Tooch_UpdateJoystick(const Bule_Tooch_Token *tokens,
														uint8_t token_count)
{
	float value[BULE_TOOCH_JOYSTICK_VALUE_NUM] = {0.0f};
	uint8_t i = 0;

	if (token_count != 5)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_FORMAT);
	}

	for (i = 0; i < BULE_TOOCH_JOYSTICK_VALUE_NUM; i++)
	{
		if (Bule_Tooch_ParseFloat(&tokens[i + 1], &value[i]) == 0)
		{
			return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_NUMBER);
		}
	}

	Bule_tooch_Data.Data_Source = 'j';
	Bule_tooch_Data.Slider_Target = 0;
	Bule_tooch_Data.Slider_Value = 0.0f;

	for (i = 0; i < BULE_TOOCH_JOYSTICK_VALUE_NUM; i++)
	{
		Bule_tooch_Data.Joystick_Value[i] = value[i];
	}
	Bule_tooch_Data.Joystick_1 = value[0];
	Bule_tooch_Data.Joystick_2 = value[1];
	Bule_tooch_Data.Joystick_3 = value[2];
	Bule_tooch_Data.Joystick_4 = value[3];

	Bule_tooch_Data.Valid = 1;
	Bule_tooch_Data.Update_Flag = 1;
	Bule_tooch_Data.Frame_Count++;

	return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_OK);
}

/**
 * @brief 处理按键协议包。
 * @param tokens 已切分的字段数组，格式应为 k,open_bule,d/u。
 * @param token_count 字段数量，按键协议必须为 3。
 * @return Bule_Tooch_ParseResult，成功时会更新 Open_Bule 和 Key_State。
 */
static Bule_Tooch_ParseResult Bule_Tooch_UpdateKey(const Bule_Tooch_Token *tokens,
												   uint8_t token_count)
{
	char state = 0;

	if (token_count != 3)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_FORMAT);
	}

	if (Bule_Tooch_TokenEqualsString(&tokens[1], "open_bule") == 0)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_KEY);
	}

	if ((Bule_Tooch_TokenIsChar(&tokens[2], 'd') == 0) &&
		(Bule_Tooch_TokenIsChar(&tokens[2], 'u') == 0))
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_KEY);
	}

	state = tokens[2].Start[0];

	Bule_tooch_Data.Data_Source = 'k';
	Bule_tooch_Data.Key_Target = 1;
	Bule_tooch_Data.Key_State = (uint8_t)state;
	Bule_tooch_Data.Open_Bule = (uint8_t)(state == 'd');

	Bule_tooch_Data.Valid = 1;
	Bule_tooch_Data.Update_Flag = 1;
	Bule_tooch_Data.Frame_Count++;

	return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_OK);
}

/**
 * @brief 解析一帧蓝牙协议数据。
 * @param buffer 接收缓冲区首地址，内容应包含完整协议包。
 * @param length 缓冲区内有效数据长度，单位为字节。
 * @return Bule_Tooch_ParseResult，返回 BULE_TOOCH_PARSE_OK 表示解析成功。
 */
Bule_Tooch_ParseResult Bule_Tooch_Parse(const uint8_t *buffer, uint16_t length)
{
	uint16_t frame_start = 0;
	uint16_t frame_end = 0;
	uint8_t token_count = 0;
	Bule_Tooch_Token tokens[BULE_TOOCH_MAX_FIELD_NUM];

	if ((buffer == 0) || (length == 0))
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_NO_FRAME);
	}

	if (Bule_Tooch_FindFrame(buffer, length, &frame_start, &frame_end) == 0)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_NO_FRAME);
	}

	if (frame_end <= (uint16_t)(frame_start + 1))
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_FORMAT);
	}

	if (Bule_Tooch_SplitTokens((const char *)&buffer[frame_start + 1],
							   (uint16_t)(frame_end - frame_start - 1),
							   tokens,
							   BULE_TOOCH_MAX_FIELD_NUM,
							   &token_count) == 0)
	{
		return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_FORMAT);
	}

	if (Bule_Tooch_TokenIsChar(&tokens[0], 's') != 0)
	{
		return Bule_Tooch_UpdateSlider(tokens, token_count);
	}

	if (Bule_Tooch_TokenIsChar(&tokens[0], 'j') != 0)
	{
		return Bule_Tooch_UpdateJoystick(tokens, token_count);
	}

	if (Bule_Tooch_TokenIsChar(&tokens[0], 'k') != 0)
	{
		return Bule_Tooch_UpdateKey(tokens, token_count);
	}

	return Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_TYPE);
}

/**
 * @brief 按字节拼接协议包，收到 ']' 后自动调用 Bule_Tooch_Parse。
 * @param data 串口收到的 1 个字节。
 * @return 无。
 * @note 当前 USART2 使用 DMA 接收，该函数保留给非 DMA 接收模式使用。
 */
void Bule_Tooch_ParseByte(uint8_t data)
{
	if (data == '[')
	{
		Bule_Tooch_RxBusy = 1;
		Bule_Tooch_RxIndex = 0;
	}

	if (Bule_Tooch_RxBusy == 0)
	{
		return;
	}

	if (Bule_Tooch_RxIndex >= BULE_TOOCH_RX_FRAME_MAX)
	{
		Bule_Tooch_RxBusy = 0;
		Bule_Tooch_RxIndex = 0;
		(void)Bule_Tooch_SaveResult(BULE_TOOCH_PARSE_BAD_FORMAT);
		return;
	}

	Bule_Tooch_RxFrame[Bule_Tooch_RxIndex] = data;
	Bule_Tooch_RxIndex++;

	if (data == ']')
	{
		(void)Bule_Tooch_Parse(Bule_Tooch_RxFrame, Bule_Tooch_RxIndex);
		Bule_Tooch_RxBusy = 0;
		Bule_Tooch_RxIndex = 0;
	}
}

/**
 * @brief 启动 USART2 的 DMA 接收。
 * @param 无。
 * @return 无。
 * @note 函数会清空 Bule_Tooch_DmaBuffer，重新启动 USART2 DMA，并打开 UART_IT_IDLE 空闲中断。
 */
void Bule_Tooch_UART2_DMA_Start(void)
{
	memset(Bule_Tooch_DmaBuffer, 0, BULE_TOOCH_DMA_BUFFER_SIZE);

	if (huart2.hdmarx != 0)
	{
		HAL_UART_DMAStop(&huart2);
		HAL_UART_Receive_DMA(&huart2, Bule_Tooch_DmaBuffer, BULE_TOOCH_DMA_BUFFER_SIZE);
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	}
}

/**
 * @brief 清除新数据标志位。
 * @param 无。
 * @return 无。
 * @note 业务代码处理完 Bule_tooch_Data 后调用。
 */
void Bule_Tooch_ClearUpdateFlag(void)
{
	Bule_tooch_Data.Update_Flag = 0;
}
