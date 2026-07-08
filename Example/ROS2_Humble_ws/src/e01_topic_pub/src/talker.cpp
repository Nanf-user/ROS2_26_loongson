#include "rclcpp/rclcpp.hpp"           // ROS 2 C++客户端库核心头文件
#include "std_msgs/msg/string.hpp"     // 标准字符串消息类型

using namespace std::chrono_literals;  // 用于时间字面量（如500ms）

/**
 * @brief ROS 2话题发布者节点
 * 
 * 该类继承自rclcpp::Node，创建一个发布者并定期发布字符串消息
 */
class Talker : public rclcpp::Node
{
public:
    /**
     * @brief 节点构造函数
     * 
     * 初始化节点名称、创建发布者和定时器
     */
    Talker() : Node("talker_node")
    {
        // 创建发布者对象
        // 模板参数：消息类型（std_msgs::msg::String）
        // 参数1：话题名称（"topic_example"）
        // 参数2：QoS（服务质量）设置，队列大小为10
        publisher_ = this->create_publisher<std_msgs::msg::String>("topic_example", 10);
        
        // 创建定时器，定期触发回调函数
        // 参数1：定时器周期（500毫秒）
        // 参数2：绑定的回调函数（Talker::timer_callback）
        timer_ = this->create_wall_timer(
            500ms, std::bind(&Talker::timer_callback, this));
            
        RCLCPP_INFO(this->get_logger(), "Talker节点已初始化");
    }

private:
    /**
     * @brief 定时器回调函数
     * 
     * 每次定时器触发时执行，生成并发布消息
     */
    void timer_callback()
    {
        // 创建消息对象
        auto message = std_msgs::msg::String();
        
        // 设置消息内容（包含计数器）
        message.data = "Hello, ROS 2! Count: " + std::to_string(count_++);
        
        // 使用ROS 2日志系统输出发布信息
        RCLCPP_INFO(this->get_logger(), "发布消息: '%s'", message.data.c_str());
        
        // 发布消息到"chatter"话题
        publisher_->publish(message);
    }

    // 成员变量
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;  // 发布者对象
    rclcpp::TimerBase::SharedPtr timer_;                              // 定时器对象
    size_t count_ = 0;                                                 // 消息计数器
};

/**
 * @brief 主函数
 * 
 * 程序入口点，初始化ROS 2并运行节点
 */
int main(int argc, char * argv[])
{
    // 初始化ROS 2上下文
    rclcpp::init(argc, argv);
    
    // 创建节点实例（使用智能指针管理生命周期）
    auto node = std::make_shared<Talker>();
    
    // 进入节点的主循环，处理回调函数
    rclcpp::spin(node);
    
    // 清理资源并关闭ROS 2
    rclcpp::shutdown();
    
    return 0;
}