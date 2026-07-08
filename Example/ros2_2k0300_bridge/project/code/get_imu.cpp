#include "get_imu.h"
#include "zf_common_headfile.h"

#include "class_posix_pit.h"
#include "get_config.h"



// 定义常量
#define DELTA_T 			0.010f


// 四元数结构体
typedef struct 
{
    float w;
    float x;
    float y;
    float z;
} Quaternion;

// 欧拉角结构体
typedef struct 
{
    float roll;
    float pitch;
    float yaw;
} EulerAngles;

// 全局变量
Quaternion q = {1.0f, 0.0f, 0.0f, 0.0f};

// 滤波后的角度，单位度
EulerAngles angles = {0};

// 滤波后的欧拉角，单位弧度
EulerAngles eular = {0};

void imu_callback()
{
	imu_acc_x = imu_get_raw(imu_file_path[ACC_X_RAW]);
	imu_acc_y = imu_get_raw(imu_file_path[ACC_Y_RAW]);
	imu_acc_z = imu_get_raw(imu_file_path[ACC_Z_RAW]);

	imu_gyro_x = imu_get_raw(imu_file_path[GYRO_X_RAW]);
	imu_gyro_y = imu_get_raw(imu_file_path[GYRO_Y_RAW]);
	// 去掉个位数，简单的去除零漂。
	imu_gyro_z = imu_get_raw(imu_file_path[GYRO_Z_RAW]) / 10 * 10;


	float gz = (float)imu_gyro_z / 16.384;

	// 通过积分获得航向角
	// angles.pitch = 0;
	// angles.roll = 0;
	angles.yaw = angles.yaw + gz * DELTA_T;


	if(angles.yaw >= 180)
	{
		angles.yaw = - 360 + angles.yaw;
	}
	else if(angles.yaw <= -180)
	{
		angles.yaw = 360 + angles.yaw;
	}
	eular.yaw = angles.yaw * M_PI / 180;
	

	pthread_mutex_lock(&g_tcp_sensor_mutex);
	g_tcp_sensor.imu.acc_x = imu_acc_x;
	g_tcp_sensor.imu.acc_y = imu_acc_y;
	g_tcp_sensor.imu.acc_z = imu_acc_z;

	g_tcp_sensor.imu.gyro_x = imu_gyro_x;
	g_tcp_sensor.imu.gyro_y = imu_gyro_y;
	g_tcp_sensor.imu.gyro_z = imu_gyro_z;

	g_tcp_sensor.imu.yaw = eular.yaw;
	pthread_mutex_unlock(&g_tcp_sensor_mutex);
	
	// pthread_mutex_unlock(&mutex);
}


void get_imu_init(void)
{

    imu_get_dev_info();
    
    if(DEV_IMU660RA == imu_type)
    {
        printf("IMU DEV IS IMU660RA\r\n");
    }
    else if(DEV_IMU660RB == imu_type)
    {
        printf("IMU DEV IS IMU660RB\r\n");
    }
    else if(DEV_IMU963RA == imu_type)
    {
        printf("IMU DEV IS IMU963RA\r\n");
    }
    else
    {
        printf("NO FIND IMU DEV\r\n");
        return ;
    }

	// ips200_init("/dev/fb0");

    // // 初始化互斥锁
    // pthread_mutex_init(&mutex, NULL);
}


void imu_thd_entry (void) 
{

    // // 尝试设置新的高优先级,值越大优先级越高
    // param.sched_priority = 10;
    // pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);


	// pthread_getschedparam(pthread_self(), &policy, &param);
    // printf("encoder_thd_entry Internal-priority thread initial priority: %d\n", param.sched_priority);
        
	// 尝试设置新的高优先级,值越大优先级越高（1-99）
	struct sched_param param;
	param.sched_priority = IMU_THD_PRO;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);


    while(1)
    {     
		// printf("imu_acc_x  = %d\r\n", imu_acc_x);
		// printf("imu_acc_y  = %d\r\n", imu_acc_y);
		// printf("imu_acc_z  = %d\r\n", imu_acc_z);

		// printf("imu_gyro_x = %d\r\n", imu_gyro_x);
		// printf("imu_gyro_y = %d\r\n", imu_gyro_y);
		// printf("imu_gyro_z = %d\r\n", imu_gyro_z);


		// pthread_mutex_lock(&mutex);
		// float temp = angles.yaw;
		// pthread_mutex_unlock(&mutex);

		// static uint8 xx;

		// ips200_show_float( 0, 0, angles.yaw, 5, 5);      
        // ips200_show_float( 0, 16*1, xx++, 5, 5);          
        // // ips200_show_float(  0, 16*2,   angles.yaw * 180.0f / PI,      5, 5);        

		// // pthread_mutex_lock(&mutex);
		// // 输出结果，单位：度
		// // printf("Roll: %f degrees\n", angles.roll);
		// // printf("Pitch: %f degrees\n", angles.pitch);
	
		// // printf("imu_thd_entry = %d\n", xx++);
		printf("yaw = %f\n", angles.yaw);
		// // // pthread_mutex_unlock(&mutex);

		// 不建议低于10ms
        system_delay_ms(10);
    }
}