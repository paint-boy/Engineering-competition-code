#include "Bule_Car_Control.h"

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
    if (Bule_tooch_Data.Data_Source == 's')
    {
        if (Bule_tooch_Data.Slider_Target == 'Y')
        {
        }
    }
    else if (Bule_tooch_Data.Data_Source == 'j')
    {
        Max_Speed_Limit(Bule_tooch_Data.Joystick_1, 5.0f);
        Max_Speed_Limit(Bule_tooch_Data.Joystick_2, 5.0f);
        Car_data.Turn_speed = Bule_tooch_Data.Joystick_1;
        Car_data.Line_speed = Bule_tooch_Data.Joystick_2;
    }
}


