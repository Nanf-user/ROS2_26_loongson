
#include "user_pid.h"

#define PWM_MAX (10000 - 1)

/* 左边电机  */
struct user_pid_t left_pid = {
        .kp = 12,
        .ki = 0.1,
        .kd = 4,
        
        .max_value = PWM_MAX,
};

/* 右边电机  */
struct user_pid_t right_pid = {
        .kp = 12,
        .ki = 0.1,
        .kd = 4,

        .max_value = PWM_MAX,
};

/* 两边电机  */
struct user_pid_t both_motor_pid = 
{
        .kp = 2.5,
        .ki = 2,
        .kd = 3,
        .max_value = PWM_MAX,
};




/* 舵机pid  */
struct user_pid_t servo_pid = {
        .kp = 0,
        .kd = 0,
};



// 增量式pid计算
void pid_inc_cal(struct user_pid_t *temp_pid, int16 set_value, int16 encoder_val)
{
    temp_pid->ek  =  set_value - (encoder_val);

    // if(temp_pid->ek > 10) temp_pid->ek = 10;

    int32 temp =  temp_pid->kp*(temp_pid->ek - temp_pid->ek_1) +
        temp_pid->ki* temp_pid->ek +
        temp_pid->kd*(temp_pid->ek - 2*temp_pid->ek_1 + temp_pid->ek_2);


    temp_pid->out_value = temp_pid->out_value + temp;

    if(temp_pid->out_value > temp_pid->max_value)
    {
        temp_pid->out_value = temp_pid->max_value;
    }
    else if(temp_pid->out_value < -temp_pid->max_value)
    {
        temp_pid->out_value = -temp_pid->max_value;
    }

    temp_pid->ek_2 = temp_pid->ek_1;
    temp_pid->ek_1 = temp_pid->ek;
}

// 位置式pid计算，返回舵机pwm值
int32 pid_pos_cal(struct user_pid_t *temp_pid, int16 prop, int16 diff)
{
    return (temp_pid->kp*prop + temp_pid->kd*diff);
}



