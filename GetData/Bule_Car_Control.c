#include "Bule_Car_Control.h"
#include "USART_Send_Cloud.h"

static float Max_Speed_Limit(float Speed, float MAX_Speed)
{
    if (Speed > MAX_Speed)
        return MAX_Speed;
    else if (Speed < -MAX_Speed)
        return -MAX_Speed;
    else
        return Speed;
}

void bule_car_key(Bule_tooch *data)
{
    static uint8_t Open_Bule_Flag = 0;
    if (Bule_tooch_Data.Update_Flag)
    {
        if (Bule_tooch_Data.Open_Bule)
        {
            Bule_Car_Control_Update(&Bule_tooch_Data);

            //要加上状态过渡
        }
        else
        {



        }

        Bule_Tooch_ClearUpdateFlag();
    }
}

void Bule_Car_Control_Update(volatile Bule_tooch *data)
{
    static uint32_t Last_Cloud_Frame_Count = 0;
    uint8_t Need_Send_Cloud = 0;

    (void)data;

    if (Bule_tooch_Data.Data_Source == 's')
    {
        if (Bule_tooch_Data.Slider_Target == 'Y')
        {
            Car_data.Yaw_Angle = Bule_tooch_Data.Yaw_angle;
        }
        else if (Bule_tooch_Data.Slider_Target == 'R')
        {
            Car_data.Rool_Angle = Bule_tooch_Data.Rool_angle;
            Need_Send_Cloud = 1;
        }
        else if (Bule_tooch_Data.Slider_Target == 'P')
        {
            Car_data.Pitch_Angle = Bule_tooch_Data.Pitch_angle;
            Need_Send_Cloud = 1;
        }
        else if (Bule_tooch_Data.Slider_Target == 'N')
        {
            Car_data.NDUN_Power = Bule_tooch_Data.NDUN;
            Need_Send_Cloud = 1;
        }

        if (Last_Cloud_Frame_Count != Bule_tooch_Data.Frame_Count)
        {
            Last_Cloud_Frame_Count = Bule_tooch_Data.Frame_Count;
            if (Need_Send_Cloud != 0U)
            {
                USART_Send_Cloud_RequestAll();
            }
        }
    }
    else if (Bule_tooch_Data.Data_Source == 'j')
    {
        Max_Speed_Limit(Bule_tooch_Data.Joystick_1, 5.0f);
        Max_Speed_Limit(Bule_tooch_Data.Joystick_2, 5.0f);
        Car_data.Turn_speed = Bule_tooch_Data.Joystick_1;
        Car_data.Line_speed = Bule_tooch_Data.Joystick_2;
    }

    (void)USART_Send_Cloud_Task();
}


