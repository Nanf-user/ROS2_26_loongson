#ifndef __base_control_h__
#define __base_control_h__

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/imu.hpp>

#include <iostream>
#include <sstream>


#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>

#include "car_point.h"

// 定义IMU数据结构体
typedef struct __attribute__((packed)) 
{
    // 航向角，弧度
    float yaw;

    // 原始数据
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;

    // 原始数据
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;

} imu_StructDef;

// 定义编码器和舵机数据结构体
typedef struct __attribute__((packed)) 
{
    int16_t encoder_left;
    int16_t encoder_right;
    int16_t servo_duty;
} encoder_and_servo_StructDef;

// 定义总的传感器数据结构体
typedef struct __attribute__((packed))
{
    imu_StructDef imu;
    encoder_and_servo_StructDef enc_servo;
}tcp_xfer_sensor_dat_TypeDef;


#define TCP_PORT 8889

class TcpServerRecvNode : public rclcpp::Node
{
public:
    TcpServerRecvNode();
    ~TcpServerRecvNode() = default;

private:
    void init_tcp_connection();
    void receive_data(int client_socket);
    void tcp_to_pub_data(sensor_msgs::msg::Imu& imu_msg, nav_msgs::msg::Odometry& odom_msg, uint8_t *buf);
    void sensor_data_init(sensor_msgs::msg::Imu& imu_msg, nav_msgs::msg::Odometry& odom_msg);

    tcp_xfer_sensor_dat_TypeDef tcp_sensor_;

    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr laser_pub_;

    std::string odom_frame_, base_frame_;

    CarState car_state_;

    sensor_msgs::msg::Imu imu_msg_;
    nav_msgs::msg::Odometry odom_msg_;
    sensor_msgs::msg::LaserScan laser_msg_;

    std::unique_ptr<tf2_ros::StaticTransformBroadcaster> static_tf_broadcaster_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    rclcpp::TimerBase::SharedPtr timer_;
    int socket_;
};


#endif