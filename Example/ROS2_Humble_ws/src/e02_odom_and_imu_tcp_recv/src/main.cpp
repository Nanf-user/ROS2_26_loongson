#include "main.h"

#define PI 3.14159265358979323846

TcpServerRecvNode::TcpServerRecvNode() : Node("odom_and_imu_tcp_recv")
{
    // 创建 IMU 数据发布者
    imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>("imu", 10);
    // 创建里程计数据发布者
    odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 10);

    // 初始化坐标变换发布
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster> (*this);

    // 初始化传感器数据
    sensor_data_init(imu_msg_, odom_msg_);

    // 初始化小车位置
    car_init_state(&car_state_);

    odom_frame_ = "odom";
    // base_frame_ = "base_link";
    base_frame_ = "base_footprint";

    // 初始化TCP连接
   
    init_tcp_connection();
}

void TcpServerRecvNode::init_tcp_connection()
{
    // 创建套接字
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == -1) 
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to create socket");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TCP_PORT);

    // 绑定套接字到指定地址和端口
    if (bind(socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to bind socket");
        close(socket_);
        return;
    }

    // 监听连接请求
    if (listen(socket_, 1) == -1) 
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to listen on socket");
        close(socket_);
        return;
    }

    RCLCPP_INFO(this->get_logger(), "Server is listening on port %d...", TCP_PORT);

    while (rclcpp::ok()) 
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        // 接受客户端连接
        int client_socket = accept(socket_, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_socket == -1) 
        {
            RCLCPP_ERROR(this->get_logger(), "Failed to accept client connection");
            // 关闭TCP
            close(client_socket);
            return;
            // continue;
        }

        // 接收数据
        this->receive_data(client_socket);

        // 关闭TCP
        close(client_socket);
    }
}

void TcpServerRecvNode::receive_data(int client_socket) 
{
    #define BUF_SIZE (sizeof(tcp_xfer_sensor_dat_TypeDef))

    uint8_t buf[BUF_SIZE];
    while (rclcpp::ok())
    {
        ssize_t recv_len = recv(client_socket, buf, BUF_SIZE, 0);
        if (recv_len <= 0) 
        {
            RCLCPP_INFO(this->get_logger(), "Client disconnected");
            break;
        }
        // RCLCPP_INFO(this->get_logger(), "recv_len = %ld", recv_len);
        if(BUF_SIZE == recv_len)
        {
            this->tcp_to_pub_data(imu_msg_, odom_msg_, buf);

            imu_pub_->publish(imu_msg_);
            odom_pub_->publish(odom_msg_);
        }
    }
}

// ACC单位为 g(m/s^2)
#define ACC_RATIO       (8 * 9.8 / 4096)
// GYRO单位弧度
#define RYRO_RATIO      ( PI / 180 / 16.384 * SAMP_DT)

void TcpServerRecvNode::tcp_to_pub_data(
    sensor_msgs::msg::Imu& imu_msg,
    nav_msgs::msg::Odometry& odom_msg,
    uint8_t *buf
)
{
    tcp_xfer_sensor_dat_TypeDef *sensor_data =  (tcp_xfer_sensor_dat_TypeDef*) buf;
    // RCLCPP_INFO(this->get_logger(), "sensor_data = %d", sensor_data->imu.acc_x);

    auto now = this->get_clock()->now();

    // --------填充IMU数据--------//
    imu_msg.header.stamp = now;

    // 将欧拉角（单位：弧度）转换为四元数
    tf2::Quaternion q_imu;
    q_imu.setRPY(0, 0, -sensor_data->imu.yaw);
    // q.setRPY(roll, pitch, yaw);

    // 示例角速度和线性加速度，GYRO单位弧度,ACC单位为 g(m/s^2)
    imu_msg.angular_velocity.x    = (float)(sensor_data->imu.gyro_x) * RYRO_RATIO;
    imu_msg.angular_velocity.y    = (float)(sensor_data->imu.gyro_y) * RYRO_RATIO;
    imu_msg.angular_velocity.z    = (float)(sensor_data->imu.gyro_z) * RYRO_RATIO;
    imu_msg.linear_acceleration.x = (float)(sensor_data->imu.acc_x) * ACC_RATIO;
    imu_msg.linear_acceleration.y = (float)(sensor_data->imu.acc_y) * ACC_RATIO;
    imu_msg.linear_acceleration.z = (float)(sensor_data->imu.acc_z) * ACC_RATIO;
    // 填充 IMU 消息的姿态信息
    imu_msg.orientation.x = q_imu.x();
    imu_msg.orientation.y = q_imu.y();
    imu_msg.orientation.z = q_imu.z();
    imu_msg.orientation.w = q_imu.w();
    // --------填充IMU数据--------//

    // 计算上一次的航向角与本次差值，获取转向角度
    float temp_theta = -sensor_data->imu.yaw - car_state_.theta;
    // // 填充航向角数据
    // car_state_.theta = sensor_data->imu.yaw;

    // --------小车X坐标、Y坐标、航向角计算--------//
    update_car_state
    (
        &car_state_, 
        sensor_data->enc_servo.encoder_left, 
        sensor_data->enc_servo.encoder_right, 
        temp_theta
    );
    // --------小车X坐标、Y坐标、航向角计算--------//

    // // 小车航向角转四元素
    // tf2::Quaternion q_car;
    // q_car.setRPY(0, 0, car_state_.theta);


    // --------填充tf变换--------//
    // odom->base坐标系
    geometry_msgs::msg::TransformStamped transform_stamped;
    transform_stamped.header.stamp = now;

    transform_stamped.header.frame_id = odom_frame_;
    transform_stamped.child_frame_id = base_frame_;

    transform_stamped.transform.translation.x = car_state_.x;
    transform_stamped.transform.translation.y = car_state_.y;
    transform_stamped.transform.translation.z = 0;

    transform_stamped.transform.rotation.x = q_imu.x();
    transform_stamped.transform.rotation.y = q_imu.y();
    transform_stamped.transform.rotation.z = q_imu.z();
    transform_stamped.transform.rotation.w = q_imu.w();

    tf_broadcaster_->sendTransform (transform_stamped);
    // --------填充tf变换--------//

    // --------填充里程计数据--------//
    odom_msg.header.stamp = now;
    odom_msg.header.frame_id = odom_frame_;
    odom_msg.child_frame_id = base_frame_;
    // 填充位置信息
    odom_msg.pose.pose.position.x = car_state_.x;
    odom_msg.pose.pose.position.y = car_state_.y;
    // odom_msg.pose.pose.position.z = 0;
    // 设置姿态信息
    odom_msg.pose.pose.orientation.x = q_imu.x();
    odom_msg.pose.pose.orientation.y = q_imu.y();
    odom_msg.pose.pose.orientation.z = q_imu.z();
    odom_msg.pose.pose.orientation.w = q_imu.w();

    // 填充线速度和角速度信息
    odom_msg.twist.twist.linear.x  = car_state_.linear_x;
    odom_msg.twist.twist.linear.y  = car_state_.linear_y;
    odom_msg.twist.twist.angular.z = car_state_.theta;           // 改动

    // // 填充里程计协方差矩阵，用于robt_pose_ekf功能包，静止和运动使用不同的参数
    // if(odom_msg.twist.twist.linear.x == 0  && odom_msg.twist.twist.linear.y == 0 & odom_msg.twist.twist.angular.z == 0)
    // {
    //     odom_msg.pose.covariance = {
    //         1e-9, 0,    0,    0,   0,   0   ,
    //         0   , 1e-3, 1e-9, 0,   0,   0   ,
    //         0   , 0,    1e6,  0,   0,   0   ,
    //         0   , 0,    0,    1e6, 0,   0   ,
    //         0   , 0,    0,    0,   1e6, 0   ,
    //         0   , 0,    0,    0,   0,   1e-9,
    //     };

    //     odom_msg.twist.covariance = {
    //         1e-9, 0,    0,    0,   0,   0   ,
    //         0   , 1e-3, 1e-9, 0,   0,   0   ,
    //         0   , 0,    1e6 , 0,   0,   0   ,
    //         0   , 0,    0,    1e6, 0,   0   ,
    //         0   , 0,    0,    0,   1e6, 0   ,
    //         0   , 0,    0,    0,   0,   1e-9,
    //     };
    // }
    // else
    // {
    //     odom_msg.pose.covariance = {
    //         1e-3, 0,    0,    0,   0,   0   ,
    //         0   , 1e-3, 0,    0,   0,   0   ,
    //         0   , 0,    1e6,  0,   0,   0   ,
    //         0   , 0,    0,    1e6, 0,   0   ,
    //         0   , 0,    0,    0,   1e6, 0   ,
    //         0   , 0,    0,    0,   0,   1e3 ,
    //     };

    //     odom_msg.twist.covariance = {
    //         1e-3, 0,    0,    0,   0,   0   ,
    //         0   , 1e-3, 1e-9, 0,   0,   0   ,
    //         0   , 0,    1e6 , 0,   0,   0   ,
    //         0   , 0,    0,    1e6, 0,   0   ,
    //         0   , 0,    0,    0,   1e6, 0   ,
    //         0   , 0,    0,    0,   0,   1e3 ,
    //     };
    // }

    // --------填充里程计数据--------//

    // RCLCPP_INFO(this->get_logger(), "car_state_.linear_x = %f"    , odom_msg.pose.pose.position.x );
    // RCLCPP_INFO(this->get_logger(), "car_state_.linear_y = %f"    , odom_msg.pose.pose.position.y);
    
    // RCLCPP_INFO(this->get_logger(), "car_state_.x = %f"    , car_state_.x);
    // RCLCPP_INFO(this->get_logger(), "car_state_.y = %f"    , car_state_.y);
    // RCLCPP_INFO(this->get_logger(), "car_state_.theta = %f", car_state_.theta);
    // RCLCPP_INFO(this->get_logger(), "sensor_data->imu.yaw = %f", sensor_data->imu.yaw);

    // RCLCPP_INFO(this->get_logger(), "imu_msg.angular_velocity.z = %f",     imu_msg.angular_velocity.z * 0.010);
    // RCLCPP_INFO(this->get_logger(), "sensor_data->imu.gyro_z = %d",     sensor_data->imu.gyro_z);
}

void TcpServerRecvNode::sensor_data_init(
    sensor_msgs::msg::Imu& imu_msg,
    nav_msgs::msg::Odometry& odom_msg)
{
    // --------填充IMU数据--------//
    imu_msg.header.frame_id = "imu_link";
    for(uint8_t i = 0; i < 9; i++)
    {
        imu_msg.orientation_covariance[i] = 1.0;
        imu_msg.angular_velocity_covariance[i] = 1.0;
        imu_msg.linear_acceleration_covariance[i] = 1.0;
    }
    // --------填充IMU数据--------//

    // --------填充里程计数据--------//
    odom_msg.header.frame_id = "odom";
    // --------填充里程计数据--------//
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TcpServerRecvNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}