#include "get_encoder.h"
#include "zf_common_headfile.h"
#include "get_config.h"

#include <stdio.h>
#include <math.h>


int16 g_encoder_left;
int16 g_encoder_right;

void encoder_callback()
{
    g_encoder_left  = encoder_get_count(ENCODER_1);
    g_encoder_right = -encoder_get_count(ENCODER_2);

    pthread_mutex_lock(&g_tcp_sensor_mutex);

	g_tcp_sensor.enc_servo.encoder_left = g_encoder_left;
	g_tcp_sensor.enc_servo.encoder_right = g_encoder_right;

	pthread_mutex_unlock(&g_tcp_sensor_mutex);
}

void encoder_thd_entry(void)
{

	// 尝试设置新的高优先级,值越大优先级越高（1-99）
	struct sched_param param;
	param.sched_priority = ENCODER_THD_PRO;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);


    while(1)
    {
        // printf("encl = %d\r\n", encoder_left);
        // printf("encr = %d\r\n", encoder_right);

        system_delay_ms(1);
    }
}

