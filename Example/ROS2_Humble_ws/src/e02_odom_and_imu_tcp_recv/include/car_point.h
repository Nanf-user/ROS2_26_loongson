#ifndef __car_point_h__
#define __car_point_h__

#include <cmath>
#include <cstdint>


#define PAI                     (3.1415926)     // 采样时间

#define SAMP_DT                 (0.01)          // 采样时间，单位秒
#define ROBOT_2D                (0.12)          // 左轮到后轮的轮距，单位米
#define ROBOT_L                 (0.155)         // 前轮到后轮的轮距，单位米

#define ENC_TO_DIS              (0.0026f)       // 编码器数据转换到距离的系数
// #define ENC_TO_ANGLE            (0.1669f)       // 编码器数据转换到角度的系数

#define SERVO_DUTY_TO_ANGLE     (1.00f)         // 舵机占空比转换到角度的系数

// -2.86 -> -3.14*2

// 定义结构体存储小车状态
struct CarState 
{
    float x;            // 当前x坐标
    float y;            // 当前y坐标
    float theta;        // 当前航向角，单位：弧度
    float linear_x;     // x轴的直线速度，单位m/s
    float linear_y;     // y轴的直线速度，单位m/s
};

// 初始化小车状态
void car_init_state(CarState *state);

// 根据编码器脉冲数计算车轮转动距离
float calc_wheel_distance(int16_t encoderCount, float samp_time);

// 根据舵机转角和左右轮距离计算小车运动后的坐标
void update_car_state(CarState *state, int16_t leftEncoderCount, int16_t rightEncoderCount, float gyro_z);

#endif // CAR_STATE_H