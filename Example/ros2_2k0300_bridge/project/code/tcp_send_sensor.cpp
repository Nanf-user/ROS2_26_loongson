
// 传输传感器数据
// IMU->六轴数据+航向角数据
// 编码器->左右编码器的值
// 舵机->转角数据

#include "zf_common_headfile.h"
#include "tcp_send_sensor.h"
#include "radar_data_protocol.h"

#include "get_radar.h"
#include "get_config.h"

tcp_xfer_sensor_dat_TypeDef g_tcp_sensor;
static int tcp_send_sensor_socket = 0;

void close_tcp_send_sensor(void)
{
    close(tcp_send_sensor_socket);
}


void tcp_send_sensor_thd_entry(void)
{
    // 尝试设置新的高优先级,值越大优先级越高（1-99）
	struct sched_param param;
	param.sched_priority = TCP_SEND_SENSOR_THD_PRO;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    int ret;

    while(1)
    {
        do
        {
            // 初始化 tcp 客户端
            tcp_send_sensor_socket = tcp_init(SERVER_IP, SEND_SENSOR_PORT);
            if(tcp_send_sensor_socket < 0)
            {
                printf("tcp_send_sensor_socket error \r\n");
                system_delay_ms(1000);
            }
            else
            {
                printf("tcp_send_sensor_socket OK \r\n");
            }
        } while (tcp_send_sensor_socket <= 0);
        
        while(1)
        {
            // TCP周期发送传感器数据
            pthread_mutex_lock(&g_tcp_sensor_mutex);
            ret = tcp_client_send_data(tcp_send_sensor_socket, (uint8_t *)&g_tcp_sensor, sizeof(tcp_xfer_sensor_dat_TypeDef));
            pthread_mutex_unlock(&g_tcp_sensor_mutex);

            if(ret < 0)
            {
                break;
            }

            system_delay_ms(10);
        }
    }
}

