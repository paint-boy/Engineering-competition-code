#include "Bule_Car_Control.h"

static float Max_Speed_Limit(float Speed,float MAX_Speed)
{
    if (Speed > MAX_Speed)
        return MAX_Speed;
    else if (Speed < -MAX_Speed)
        return -MAX_Speed;
    else
        return Speed;
}



void Bule_Car_Control_Update(Bule_tooch *data)
{
    if (Bule_tooch_Data.Update_Flag)
    {
        if (Bule_tooch_Data.Data_Source == 's')
        {
            if (Bule_tooch_Data.Slider_Target == 'Y')
            {



                
            }
        }
        else if (Bule_tooch_Data.Data_Source == 'j')
        {
            Max_Speed_Limit(Bule_tooch_Data.Joystick_1, 2.0f);
            Max_Speed_Limit(Bule_tooch_Data.Joystick_2, 2.0f);
            Car_data.Turn_speed = Bule_tooch_Data.Joystick_1;
            Car_data.Line_speed= -Bule_tooch_Data.Joystick_2;
        }

        Bule_Tooch_ClearUpdateFlag();
    }
}


