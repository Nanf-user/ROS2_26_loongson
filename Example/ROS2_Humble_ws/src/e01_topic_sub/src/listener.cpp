#include "rclcpp/rclcpp.hpp"           // ROS 2 C++客户端库核心头文件
#include "std_msgs/msg/string.hpp"     // 标准字符串消息类型

/**
 * @brief ROS 2话题订阅者节点
 * 
 * 该类继承自rclcpp::Node，创建一个订阅者并处理接收到的消息
 */
class Listener : public rclcpp::Node
{
public:
    /**
     * @brief 节点构造函数
     * 
     * 初始化节点名称、创建订阅者
     */
    Listener() : Node("listener_node")
    {
        // 创建订阅者对象
        // 模板参数：消息类型（std_msgs::msg::String）
        // 参数1：话题名称（"topic_example"）
        // 参数2：QoS（服务质量）设置，队列大小为10
        // 参数3：绑定的回调函数（Listener::topic_callback）
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "topic_example", 10, std::bind(&Listener::topic_callback, this, std::placeholders::_1));
            
        RCLCPP_INFO(this->get_logger(), "Listener节点已初始化，正在订阅话题 'topic_example'");
    }

private:
    /**
     * @brief 话题回调函数
     * 
     * 每当接收到新消息时执行，处理并打印接收到的消息
     * 
     * @param msg 接收到的消息指针
     */
    void topic_callback(const std_msgs::msg::String::SharedPtr msg) const
    {
        // 使用ROS 2日志系统输出接收到的消息内容
        RCLCPP_INFO(this->get_logger(), "接收到消息: '%s'", msg->data.c_str());
    }

    // 成员变量：订阅者对象
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
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
    auto node = std::make_shared<Listener>();
    
    // 进入节点的主循环，处理回调函数
    rclcpp::spin(node);
    
    // 清理资源并关闭ROS 2
    rclcpp::shutdown();
    
    return 0;
}