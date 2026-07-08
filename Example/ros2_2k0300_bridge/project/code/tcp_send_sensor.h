/*********************************************************************************************************************
* LS2K0300 Opensourec Library 即（LS2K0300 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是LS2K0300 开源库的一部分
*
* LS2K0300 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          zf_common_font
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MounRiver Studio V1.8.1
* 适用平台          LS2K0300
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期                                      作者                             备注
* 2022-09-15        大W            first version
********************************************************************************************************************/

#ifndef _tcp_send_sensor_h_
#define _tcp_send_sensor_h_

#include "zf_common_typedef.h"
#include "radar_data_protocol.h"


typedef struct __attribute__((packed)) 
{
    // // 四元素数据
    // float qx;
    // float qy;
    // float qz;
    // float qw;

    // 航向角，弧度
    // float roll;
    // float pitch;
    float yaw;

    // 单位米每秒（m/s）
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;

    // 单位为弧度每秒（rad/s）
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

} imu_StructDef;

typedef struct __attribute__((packed)) 
{
    int16_t encoder_left;
    int16_t encoder_right;
    int16_t servo_duty;
} encoder_and_servo_StructDef;

typedef struct __attribute__((packed))
{
    imu_StructDef imu;
    encoder_and_servo_StructDef enc_servo;
}tcp_xfer_sensor_dat_TypeDef;

extern pthread_mutex_t g_tcp_sensor_mutex;

extern tcp_xfer_sensor_dat_TypeDef g_tcp_sensor;

void tcp_send_sensor_thd_entry(void);
void close_tcp_send_sensor(void);


#endif
