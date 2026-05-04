#ifndef __BULE_TOOCH_H__
#define __BULE_TOOCH_H__

#include "stdint.h"

#define BULE_TOOCH_JOYSTICK_VALUE_NUM 4
#define BULE_TOOCH_DMA_BUFFER_SIZE 64

/* 蓝牙协议解析结果，用于判断当前协议包是否解析成功。 */
typedef enum
{
	BULE_TOOCH_PARSE_OK = 0,       /* 解析成功 */
	BULE_TOOCH_PARSE_NO_FRAME,     /* 没有找到完整的 [ ] 协议包 */
	BULE_TOOCH_PARSE_BAD_FORMAT,   /* 字段数量或逗号格式错误 */
	BULE_TOOCH_PARSE_BAD_TYPE,     /* 第一个字段不是 s 或 j */
	BULE_TOOCH_PARSE_BAD_TARGET,   /* 滑杆目标不是 Y/R/P/N */
	BULE_TOOCH_PARSE_BAD_NUMBER    /* 浮点数字段格式错误 */
} Bule_Tooch_ParseResult;

/* 蓝牙解析后的数据结构体，外部直接读取全局变量 Bule_tooch_Data 即可。 */
typedef struct Bule_tooch
{
	uint8_t Valid;          /* 已经成功解析过至少一帧数据 */
	uint8_t Update_Flag;    /* 新数据标志位，外部读取后调用 Bule_Tooch_ClearUpdateFlag 清零 */
	uint8_t Data_Source;    /* 数据来源：'s' 表示滑杆，'j' 表示摇杆 */
	uint8_t Slider_Target;  /* 滑杆控制目标：Y=Yaw，R=Rool，P=Pitch，N=NDUN */

	float Slider_Value;     /* 本次滑杆发送的浮点值 */
	float Yaw_angle;        /* 最近一次 [s,Y,value] 的 value */
	float Rool_angle;       /* 最近一次 [s,R,value] 的 value */
	float Pitch_angle;      /* 最近一次 [s,P,value] 的 value */
	float NDUN;             /* 最近一次 [s,N,value] 的 value */
	float Joystick_Value[BULE_TOOCH_JOYSTICK_VALUE_NUM]; /* 摇杆 4 个浮点值数组 */
	float Joystick_1;       /* 摇杆第 1 个浮点值 */
	float Joystick_2;       /* 摇杆第 2 个浮点值 */
	float Joystick_3;       /* 摇杆第 3 个浮点值 */
	float Joystick_4;       /* 摇杆第 4 个浮点值 */

	uint32_t Frame_Count;   /* 成功解析的协议包数量 */
	uint32_t Error_Count;   /* 解析失败的协议包数量 */
	Bule_Tooch_ParseResult Last_Result; /* 最近一次解析结果 */
} Bule_tooch;

/* 蓝牙协议解析后的全局数据，业务代码读取这个变量。 */
extern volatile Bule_tooch Bule_tooch_Data;

/* USART2 DMA 接收缓冲区，长度为 BULE_TOOCH_DMA_BUFFER_SIZE。 */
extern uint8_t Bule_Tooch_DmaBuffer[BULE_TOOCH_DMA_BUFFER_SIZE];

/**
 * @brief 解析一帧蓝牙协议数据。
 * @param buffer 接收缓冲区首地址，内容应包含完整协议包，例如 [s,Y,190.33] 或 [j,1.88,0.68,0,0]。
 * @param length 缓冲区内有效数据长度，单位为字节。
 * @return Bule_Tooch_ParseResult，返回 BULE_TOOCH_PARSE_OK 表示解析成功。
 */
Bule_Tooch_ParseResult Bule_Tooch_Parse(const uint8_t *buffer, uint16_t length);

/**
 * @brief 按字节拼接并解析蓝牙协议包。
 * @param data 串口收到的 1 个字节。
 * @note 该函数用于非 DMA 字节接收模式；当前 USART2 已使用 DMA，正常情况下不需要外部调用。
 */
void Bule_Tooch_ParseByte(uint8_t data);

/**
 * @brief 启动 USART2 DMA 接收蓝牙协议数据。
 * @param 无。
 * @return 无。
 * @note 在 main.c 初始化 USART2 和 DMA 后调用一次即可，函数会使用 Bule_Tooch_DmaBuffer 作为 DMA 接收缓冲区，并打开串口空闲中断。
 */
void Bule_Tooch_UART2_DMA_Start(void);

/**
 * @brief 清除蓝牙新数据标志位。
 * @param 无。
 * @return 无。
 * @note 业务代码处理完 Bule_tooch_Data 后调用，避免重复处理同一帧数据。
 */
void Bule_Tooch_ClearUpdateFlag(void);

#endif
