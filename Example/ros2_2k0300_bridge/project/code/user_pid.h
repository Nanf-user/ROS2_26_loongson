#ifndef __user_pid_h
#define __user_pid_h

#include "zf_common_headfile.h"

struct user_pid_t
{

    float kp;
    float ki;
    float kd;              // 比例、积分、微分系数


    int32 out_value;             // 输出值
    int32 max_value;             // 最大值


    int32 ek;                    // 偏差值
    int32 ek_1;                  // 上一个偏差值
    int32 ek_2;                  // 最上前的偏差值


};

void pid_inc_cal(struct user_pid_t *temp_pid, int16 set_value, int16 encoder_val);

int32 pid_pos_cal(struct user_pid_t *temp_pid, int16 prop, int16 diff);


extern struct user_pid_t left_pid;
extern struct user_pid_t right_pid;
extern struct user_pid_t both_motor_pid;

#endif








