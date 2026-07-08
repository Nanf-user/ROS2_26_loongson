#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>


#define PORT 8890

#define     func_limit(x, y)        ((x) > (y) ? (y) : ((x) < -(y) ? -(y) : (x)))

typedef struct __attribute__((packed)) 
{
    int16_t left_speed;
    int16_t right_speed;
    int16_t servo_duty;
} car_control_typedef;



class CmdVelTCPServer : public rclcpp::Node
{
public:
    CmdVelTCPServer() : Node("cmd_vel_tcp_server")
    {
        // 订阅 /cmd_vel 话题
        subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10, std::bind(&CmdVelTCPServer::cmd_vel_callback, this, std::placeholders::_1));

        init_tcp_connection();
    }

    ~CmdVelTCPServer()
    {
        if (new_socket_ != -1) {
            close(new_socket_);
        }
        if (server_fd_ != -1) {
            close(server_fd_);
        }
    }

private:


    // 初始化TCP连接
    void init_tcp_connection()
    {
        // 创建 TCP 套接字
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ == -1) {
            RCLCPP_ERROR(this->get_logger(), "Failed to create socket");
            return;
        }

        // 配置服务器地址
        server_address_.sin_family = AF_INET;
        server_address_.sin_addr.s_addr = INADDR_ANY;
        server_address_.sin_port = htons(PORT);

        // 绑定套接字到指定地址和端口
        if (bind(server_fd_, (struct sockaddr *)&server_address_, sizeof(server_address_)) == -1) {
            RCLCPP_ERROR(this->get_logger(), "Failed to bind socket");
            close(server_fd_);
            return;
        }

        // 开始监听连接
        if (listen(server_fd_, 1) == -1) {
            RCLCPP_ERROR(this->get_logger(), "Failed to listen on socket");
            close(server_fd_);
            return;
        }

        RCLCPP_INFO(this->get_logger(), "TCP server is listening on port 8890");

        // 接受客户端连接
        int addrlen = sizeof(client_address_);
        new_socket_ = accept(server_fd_, (struct sockaddr *)&client_address_, (socklen_t*)&addrlen);
        if (new_socket_ == -1) {
            RCLCPP_ERROR(this->get_logger(), "Failed to accept client connection");
            close(server_fd_);
            return;
        }

        RCLCPP_INFO(this->get_logger(), "Client connected");



 
    }


    #define  AKM_TURN_R_MINI        0.15f      // 最小转弯半径( L*cot30-W/2)
    #define  AKM_ACLE_BASE          0.155f     // 轴距，前后轮的距离
    #define  AKM_WHEEL_BASE         0.120f	   // 轮距，左右轮的距离
    #define  PI                     3.1415926f


    /**
     * @简  述  根据阿克曼前进和转向速度转换为前轮转向角度
     * @参  数  vx  前进速度，单位m/s 
     *          vw  转向速度，单位rad/s
     * @返回值  右前轮转向角度，单位rad
     */
    float AX_AKM_WToAngle(float vx, float vw) 
    {
        float akm_angle; //前轮转向角度
        float radius;    //转弯半径

        if(vw!=0 && vx!=0)
        {
            //计算转弯半径
            radius =  (float)vx/(float)vw;
            
            //转弯半径小于最小转弯
            if(radius>0 && radius<AKM_TURN_R_MINI)
            {
                radius = AKM_TURN_R_MINI; 
            }
                
            if(radius<0 && radius>(-AKM_TURN_R_MINI))
            {
                radius = -AKM_TURN_R_MINI;
            }		

            //计算机器人前轮转向角度,单位弧度
            akm_angle = atan(AKM_ACLE_BASE/(radius));
        }
        else
        {
            akm_angle = 0;
        }

        return (akm_angle);
    }


    // 计算β值，θ需以弧度制传入
    double calculate_beta(double theta) 
    {
        double denominator = 0.155 - 0.06 * tan(theta);
        if (fabs(denominator) < 1e-10) {  // 防止分母接近0
            fprintf(stderr, "分母接近0，计算错误\n");
            return -1;  // 返回特殊值表示错误
        }
        double ratio = tan(theta) / denominator;
        return atan(ratio);
    }

    void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        if (new_socket_ != -1) 
        {
            car_control_typedef car_control;

            // 正数舵机向左
            // 负数舵机向右

            // z 轴：正方向表示绕 z 轴做逆时针旋转（即向左转），负方向表示绕 z 轴做顺时针旋转（即向右转）。
            // 若msg->angular.z为正（如0.5），temp_point就为正，机器人向左转
            // 若msg->angular.z为负（如-0.5），temp_point就为负，机器人向右转
            
            float akm_angle;    // 前轮转向角度
            float radius;       // 转弯半径

            // 机器人目标速度限制
            akm_angle = AX_AKM_WToAngle(msg->linear.x, msg->angular.z);

            // 计算转弯半径
            radius =  AKM_ACLE_BASE/tan(akm_angle);
            
            float temp_left_speed  = 0;
            float temp_right_speed = 0;

            //根据前轮角度计算右前轮角度
            if(akm_angle !=0 )
            {

                //运动学逆解析，由机器人目标速度计算电机轮子速度（m/s）
                temp_left_speed  = msg->linear.x * (radius - 0.5 * AKM_WHEEL_BASE) / radius;
                temp_right_speed = msg->linear.x * (radius + 0.5 * AKM_WHEEL_BASE) / radius;					
                
       
            }
            else
            {
                //运动学逆解析，由机器人目标速度计算电机轮子速度（m/s）
                temp_left_speed  = msg->linear.x;
                temp_right_speed = msg->linear.x;	
            }

            // 速度值50    路程1.2米  4.22秒  0.28米/s
            // 速度值100   路程1.2米  2.27秒  0.53米/s
            // 速度值150   路程2.4米  3.02秒  0.79米/s
            // 速度值180   路程1.8米  2.00秒  0.90米/s

            // 将上面的数据，进行线性拟合，得到下面的公式。
            // g_car_control.speed = linear_x * 189。
            car_control.left_speed = temp_left_speed * 189;
            car_control.right_speed = temp_right_speed * 189;

            // duty = 574       radius = 0.1875 米     akm_angle = 0.6931    
            // duty = 530       radius = 0.19   米     akm_angle = 0.6876    
            // duty = 463       radius = 0.2325 米     akm_angle = 0.5925    
            // duty = 415       radius = 0.2625 米     akm_angle = 0.5397    
            // duty = 384       radius = 0.305  米     akm_angle = 0.4736    
            // duty = 338       radius = 0.365  米     akm_angle = 0.4007    
            // duty = 296       radius = 0.415  米     akm_angle = 0.3541    
            // duty = 292       radius = 0.425  米     akm_angle = 0.3469    
            // duty = 242       radius = 0.5825 米     akm_angle = 0.2537    
            // duty = 219       radius = 0.66   米     akm_angle = 0.2253    
            // duty = 198       radius = 0.705  米     akm_angle = 0.2124    

            // 将上面的数据，进行线性拟合，得到下面的公式。
            #define A  710.77
            #define B  50.344
            if(akm_angle > 0)
            {
                car_control.servo_duty = A*akm_angle + B;
            }
            else if(akm_angle < 0)
            {
                car_control.servo_duty = A*akm_angle - B;
            }
            else
            {
                car_control.servo_duty = 0;
            }


            car_control.right_speed = func_limit(car_control.right_speed, 100);
            car_control.left_speed = func_limit(car_control.left_speed, 100);

            RCLCPP_INFO(this->get_logger(), "linear: x=%f m/s angular: z=%f rad",msg->linear.x, msg->angular.z);
            RCLCPP_INFO(this->get_logger(), "radius = %f", radius); // 这个是半径，不是直径。
            RCLCPP_INFO(this->get_logger(), "akm_angle = %f", akm_angle);

            RCLCPP_INFO(this->get_logger(), "car_control.servo_duty = %d", car_control.servo_duty);
            RCLCPP_INFO(this->get_logger(), "car_control.left_speed = %d", car_control.left_speed);
            RCLCPP_INFO(this->get_logger(), "car_control.right_speed = %d", car_control.right_speed);

       
            uint8_t *buff = (uint8_t *)&car_control;
            ssize_t send_len = send(new_socket_, buff, sizeof(car_control_typedef), 0);
            if (send_len <= 0) 
            {
                close(new_socket_);
                new_socket_ = -1;
                RCLCPP_INFO(this->get_logger(), "Client disconnected");
                init_tcp_connection();
            }
        }
    }

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
    int server_fd_ = -1;
    int new_socket_ = -1;
    struct sockaddr_in server_address_;
    struct sockaddr_in client_address_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CmdVelTCPServer>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}