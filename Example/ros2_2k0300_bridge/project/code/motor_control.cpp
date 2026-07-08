#include "motor_control.h"
#include "zf_common_headfile.h"
#include "get_config.h"




void servo_control(int16 servo_duty)
{
    #define SERVO_MID       (4436)
    #define SERVO_MAX       (600)
    
    servo_duty = func_limit(servo_duty, SERVO_MAX);

    pthread_mutex_lock(&g_tcp_sensor_mutex);
	g_tcp_sensor.enc_servo.servo_duty = servo_duty;
	pthread_mutex_unlock(&g_tcp_sensor_mutex);
    
    pwm_set_duty(SERVO_MOTOR1_PWM, SERVO_MID + servo_duty);
}


#define PWM_MAX 4000
void pwm_control(int16 left_duty, int16 right_duty)
{
    left_duty = func_limit(left_duty, PWM_MAX);
    right_duty = func_limit(right_duty, PWM_MAX);

    // if(count >= 50)
    // {
    //     pwm_set_duty(MOTOR1_PWM, 0);
    //     pwm_set_duty(MOTOR2_PWM, 0);
    //     return;
    // }

    // if(left_duty == PWM_MAX || right_duty == PWM_MAX)
    // {
    //     count++;
    // }
    // else
    // {
    //     count = 0;
    // }
    // printf("count = %d\r\n", count);


    if(left_duty > 0)
    {
        gpio_set_level(MOTOR1_DIR, 1);                             
        pwm_set_duty(MOTOR1_PWM, left_duty);      
    }
    else
    {
        gpio_set_level(MOTOR1_DIR, 0);                             
        pwm_set_duty(MOTOR1_PWM, -left_duty);      
    }
    
    if(right_duty > 0)
    {
        gpio_set_level(MOTOR2_DIR, 1);                             
        pwm_set_duty(MOTOR2_PWM, right_duty);      
    }
    else
    {
        gpio_set_level(MOTOR2_DIR, 0);                             
        pwm_set_duty(MOTOR2_PWM, -right_duty);      
    }

}