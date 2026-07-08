#ifndef SIMPLE_PURSUIT_CONTROLLER__SIMPLE_PURSUIT_CONTROLLER_HPP_
#define SIMPLE_PURSUIT_CONTROLLER__SIMPLE_PURSUIT_CONTROLLER_HPP_

#include <memory>
#include <string>
#include <vector>

#include "nav2_core/controller.hpp"
#include "nav2_costmap_2d/costmap_2d_ros.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/buffer.h"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/path.hpp"
#include "std_msgs/msg/bool.hpp"

namespace simple_pursuit_controller
{

class SimplePursuitController : public nav2_core::Controller
{
public:
  SimplePursuitController() = default;
  ~SimplePursuitController() override = default;

  void configure(
    const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
    std::string name,
    std::shared_ptr<tf2_ros::Buffer> tf,
    std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros) override;

  void activate() override;
  void deactivate() override;
  void cleanup() override;

  void setPlan(const nav_msgs::msg::Path & path) override;
  void setSpeedLimit(const double & speed_limit, const bool & percentage) override;

  geometry_msgs::msg::TwistStamped computeVelocityCommands(
    const geometry_msgs::msg::PoseStamped & pose,
    const geometry_msgs::msg::Twist & velocity,
    nav2_core::GoalChecker * goal_checker) override;

private:
  // 配置参数
  double lookahead_dist_;
  double desired_linear_vel_;
  double max_linear_vel_;
  double min_linear_vel_;
  double max_angular_vel_;
  double curvature_penalty_gain_;
  double curvature_penalty_cap_;
  double goal_align_gain_;

  // 导航相关
  rclcpp_lifecycle::LifecycleNode::WeakPtr node_;
  std::string name_;
  std::shared_ptr<tf2_ros::Buffer> tf_;
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros_;
  nav_msgs::msg::Path global_plan_;
  size_t global_plan_idx_ = 0;

  // 发布者与限频
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr path_blocked_pub_;
  rclcpp::Time last_publish_time_;

  // 辅助函数
  size_t findNearestPathPoint(const geometry_msgs::msg::PoseStamped & robot_pose);
  geometry_msgs::msg::PoseStamped findLookaheadPoint(
    const geometry_msgs::msg::PoseStamped & robot_pose,
    size_t nearest_idx);

  bool isPathBlocked(const geometry_msgs::msg::PoseStamped & robot_pose);
};

}  // namespace simple_pursuit_controller

#endif  // SIMPLE_PURSUIT_CONTROLLER__SIMPLE_PURSUIT_CONTROLLER_HPP_