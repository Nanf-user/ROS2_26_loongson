#include "car_point.h"

#include <cstdio>

// 初始化小车状态
void car_init_state(CarState *state) 
{
    state->x = 0.0;
    state->y = 0.0;
    state->theta = 0.0;
    state->linear_x = 0.0;
    state->linear_y = 0.0;
}

// 根据编码器脉冲数计算车轮转动距离
float calc_wheel_distance(int16_t encoderCount, float samp_time) 
{
    return static_cast<float>(encoderCount) * ENC_TO_DIS * samp_time;
}

// 根据舵机转角和左右轮距离计算小车运动后的坐标
void update_car_state(CarState *state, int16_t leftEncoderCount, int16_t rightEncoderCount, float gyro_z) 
{
    double delta_time_ = SAMP_DT;
    double delta_right = rightEncoderCount * delta_time_;
    double delta_left  = leftEncoderCount  * delta_time_;

    double delta_dis = (delta_right + delta_left) * ENC_TO_DIS;
    // double v_dis = delta_dis / delta_time_;

    double delta_theta = gyro_z;//(delta_right - delta_left) * ENC_TO_ANGLE;
    // double v_theta = delta_theta / delta_time_;
    double delta_x, delta_y;


    // 小于0.02弧度认为没有转向
    if (std::fabs(delta_theta) < 0.02)
    {
        delta_x = delta_dis;
        delta_y = 0.0;
    }
    else
    {
        delta_x = delta_dis * (sin (delta_theta) / delta_theta);
        delta_y = delta_dis * ( (1 - cos (delta_theta) ) / delta_theta);
    }

    state->x += (cos (state->theta) * delta_x - sin (state->theta) * delta_y);
    state->y += (sin (state->theta) * delta_x + cos (state->theta) * delta_y);
    state->theta += delta_theta;

    state->linear_x = (cos (state->theta) * delta_x - sin (state->theta) * delta_y) / SAMP_DT;
    state->linear_y = (sin (state->theta) * delta_x + cos (state->theta) * delta_y) / SAMP_DT;

}