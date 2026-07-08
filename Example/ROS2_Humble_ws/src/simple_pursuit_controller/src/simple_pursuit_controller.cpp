#include "simple_pursuit_controller/simple_pursuit_controller.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include "tf2/utils.h"
#include "nav2_costmap_2d/costmap_2d.hpp"
#include "nav2_costmap_2d/cost_values.hpp"
#include "pluginlib/class_list_macros.hpp"

namespace simple_pursuit_controller
{

void SimplePursuitController::configure(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
  std::string name,
  std::shared_ptr<tf2_ros::Buffer> tf,
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros)
{
  node_ = parent;
  name_ = name;
  tf_ = tf;
  costmap_ros_ = costmap_ros;

  auto node = node_.lock();
  if (!node) {
    throw std::runtime_error("Failed to lock node");
  }

  // 声明参数
  node->declare_parameter(name + ".lookahead_dist", 0.25);
  node->declare_parameter(name + ".desired_linear_vel", 0.15);
  node->declare_parameter(name + ".max_linear_vel", 0.25);
  node->declare_parameter(name + ".min_linear_vel", 0.05);
  node->declare_parameter(name + ".max_angular_vel", 1.8);
  node->declare_parameter(name + ".curvature_penalty_gain", 0.03);
  node->declare_parameter(name + ".curvature_penalty_cap", 0.5);
  node->declare_parameter(name + ".goal_align_gain", 2.0);

  // 获取参数
  lookahead_dist_ = node->get_parameter(name + ".lookahead_dist").as_double();
  desired_linear_vel_ = node->get_parameter(name + ".desired_linear_vel").as_double();
  max_linear_vel_ = node->get_parameter(name + ".max_linear_vel").as_double();
  min_linear_vel_ = node->get_parameter(name + ".min_linear_vel").as_double();
  max_angular_vel_ = node->get_parameter(name + ".max_angular_vel").as_double();
  curvature_penalty_gain_ = node->get_parameter(name + ".curvature_penalty_gain").as_double();
  curvature_penalty_cap_ = node->get_parameter(name + ".curvature_penalty_cap").as_double();
  goal_align_gain_ = node->get_parameter(name + ".goal_align_gain").as_double();

  // 创建发布者
  path_blocked_pub_ = node->create_publisher<std_msgs::msg::Bool>("path_blocked", 10);
  last_publish_time_ = rclcpp::Time(0, 0, RCL_ROS_TIME);

  RCLCPP_INFO(
    node->get_logger(),
    "SimplePursuitController configured: lookahead=%.2f, vel=%.2f, publisher created on /path_blocked",
    lookahead_dist_, desired_linear_vel_);
}

void SimplePursuitController::activate()
{
  RCLCPP_INFO(rclcpp::get_logger("SimplePursuitController"), "Activated");
}

void SimplePursuitController::deactivate()
{
  RCLCPP_INFO(rclcpp::get_logger("SimplePursuitController"), "Deactivated");
}

void SimplePursuitController::cleanup()
{
  path_blocked_pub_.reset();
  RCLCPP_INFO(rclcpp::get_logger("SimplePursuitController"), "Cleaned up");
}

void SimplePursuitController::setPlan(const nav_msgs::msg::Path & path)
{
  global_plan_ = path;
  global_plan_idx_ = 0;
}

void SimplePursuitController::setSpeedLimit(
  const double & speed_limit, const bool & percentage)
{
  if (percentage) {
    desired_linear_vel_ = max_linear_vel_ * speed_limit / 100.0;
  } else {
    desired_linear_vel_ = std::min(speed_limit, max_linear_vel_);
  }
  desired_linear_vel_ = std::max(desired_linear_vel_, min_linear_vel_);
}

// 检查路径前方 0.10~0.15m 处是否被障碍物占据
bool SimplePursuitController::isPathBlocked(const geometry_msgs::msg::PoseStamped & robot_pose)
{
  auto costmap = costmap_ros_->getCostmap();
  if (!costmap) {
    return false;
  }

  const double min_check_dist = 0.20;
  const double max_check_dist = 0.25;

  // 从当前最近路径点开始向后搜索
  for (size_t i = global_plan_idx_; i < global_plan_.poses.size(); ++i) {
    double dx = global_plan_.poses[i].pose.position.x - robot_pose.pose.position.x;
    double dy = global_plan_.poses[i].pose.position.y - robot_pose.pose.position.y;
    double dist = std::sqrt(dx * dx + dy * dy);

    if (dist >= min_check_dist && dist <= max_check_dist) {
      unsigned int mx, my;
      if (costmap->worldToMap(global_plan_.poses[i].pose.position.x,
                              global_plan_.poses[i].pose.position.y,
                              mx, my)) {
        unsigned char cost = costmap->getCost(mx, my);
        if (cost >= nav2_costmap_2d::INSCRIBED_INFLATED_OBSTACLE) {
          return true;
        }
      }
      // 只检查一个点，若该点畅通则路径可用
      break;
    }
  }

  // 如果路径太短（终点距离小于 min_check_dist），不触发阻挡
  if (!global_plan_.poses.empty()) {
    double dx = global_plan_.poses.back().pose.position.x - robot_pose.pose.position.x;
    double dy = global_plan_.poses.back().pose.position.y - robot_pose.pose.position.y;
    double dist = std::sqrt(dx * dx + dy * dy);
    if (dist < min_check_dist) {
      return false;
    }
  }

  return false;
}

geometry_msgs::msg::TwistStamped SimplePursuitController::computeVelocityCommands(
  const geometry_msgs::msg::PoseStamped & pose,
  const geometry_msgs::msg::Twist & /*velocity*/,
  nav2_core::GoalChecker * /*goal_checker*/)
{
  auto node = node_.lock();
  if (!node) {
    geometry_msgs::msg::TwistStamped cmd_vel;
    cmd_vel.header.stamp = rclcpp::Clock().now();
    cmd_vel.header.frame_id = "base_link";
    cmd_vel.twist.linear.x = 0.0;
    cmd_vel.twist.angular.z = 0.0;
    return cmd_vel;
  }

  // ----- 障碍物检测：检查路径前方 0.10~0.15m 处 -----
  bool path_blocked = isPathBlocked(pose);

  // ----- 以 10Hz 频率发布状态并打印 -----
  if (path_blocked_pub_) {
    rclcpp::Time now = node->now();
    if ((now - last_publish_time_).seconds() >= 0.1) {
      std_msgs::msg::Bool msg;
      msg.data = path_blocked;
      path_blocked_pub_->publish(msg);
      last_publish_time_ = now;

      // 每次发布都打印当前状态
      RCLCPP_INFO(node->get_logger(), "path_blocked: %s", path_blocked ? "true" : "false");
    }
  }

  // 如果被阻挡，停车
  if (path_blocked) {
    geometry_msgs::msg::TwistStamped cmd_vel;
    cmd_vel.header.stamp = rclcpp::Clock().now();
    cmd_vel.header.frame_id = "base_link";
    cmd_vel.twist.linear.x = 0.0;
    cmd_vel.twist.angular.z = 0.0;
    return cmd_vel;
  }

  // ----- 原有 Pure Pursuit 逻辑（不变） -----
  // 1. 找最近点
  auto nearest_idx = findNearestPathPoint(pose);

  // 2. 找前视点
  auto lookahead_point = findLookaheadPoint(pose, nearest_idx);

  // 3. 计算相对位置
  double robot_yaw = tf2::getYaw(pose.pose.orientation);
  double dx = lookahead_point.pose.position.x - pose.pose.position.x;
  double dy = lookahead_point.pose.position.y - pose.pose.position.y;

  // 转换到机器人坐标系
  double local_y = -dx * std::sin(robot_yaw) + dy * std::cos(robot_yaw);

  // 4. 计算前视距离
  double actual_lookahead = std::sqrt(dx * dx + dy * dy);

  // 5. Pure Pursuit曲率计算
  double curvature = 0.0;
  if (actual_lookahead > 0.01) {
    curvature = 2.0 * local_y / (actual_lookahead * actual_lookahead);
  }

  // 6. 计算速度
  geometry_msgs::msg::TwistStamped cmd_vel;
  cmd_vel.header.stamp = rclcpp::Clock().now();
  cmd_vel.header.frame_id = "base_link";

  // 弯道适度减速
  double curvature_abs = std::abs(curvature);
  double curvature_factor = 1.0 - std::min(curvature_abs * curvature_penalty_gain_, curvature_penalty_cap_);
  double linear_vel = desired_linear_vel_ * curvature_factor;
  linear_vel = std::clamp(linear_vel, min_linear_vel_, max_linear_vel_);

  // 角速度基于期望线速度
  double angular_vel = desired_linear_vel_ * curvature;

  // 7. 到达目标时停止
  double goal_dx = global_plan_.poses.back().pose.position.x - pose.pose.position.x;
  double goal_dy = global_plan_.poses.back().pose.position.y - pose.pose.position.y;
  double dist_to_goal = std::sqrt(goal_dx * goal_dx + goal_dy * goal_dy);

  if (dist_to_goal < 0.05) {
    cmd_vel.twist.linear.x = 0.0;
    cmd_vel.twist.angular.z = 0.0;
    return cmd_vel;
  }

  // 接近目标时混合朝向对齐 + 减速
  if (dist_to_goal < lookahead_dist_ * 1.5) {
    double goal_yaw = std::atan2(goal_dy, goal_dx);
    double yaw_error = goal_yaw - robot_yaw;
    yaw_error = std::atan2(std::sin(yaw_error), std::cos(yaw_error));

    double blend_ratio = 1.0 - std::min(dist_to_goal / (lookahead_dist_ * 1.5), 1.0);
    angular_vel = (1.0 - blend_ratio) * angular_vel + blend_ratio * goal_align_gain_ * yaw_error;

    if (dist_to_goal < 0.15) {
      linear_vel *= (dist_to_goal / 0.15);
    }
  }

  angular_vel = std::clamp(angular_vel, -max_angular_vel_, max_angular_vel_);
  linear_vel = std::clamp(linear_vel, 0.0, max_linear_vel_);

  cmd_vel.twist.linear.x = linear_vel;
  cmd_vel.twist.angular.z = angular_vel;

  return cmd_vel;
}

size_t SimplePursuitController::findNearestPathPoint(
  const geometry_msgs::msg::PoseStamped & robot_pose)
{
  double min_dist = std::numeric_limits<double>::max();
  size_t nearest_idx = global_plan_idx_;

  for (size_t i = global_plan_idx_; i < global_plan_.poses.size(); ++i) {
    double dx = global_plan_.poses[i].pose.position.x - robot_pose.pose.position.x;
    double dy = global_plan_.poses[i].pose.position.y - robot_pose.pose.position.y;
    double dist = dx * dx + dy * dy;

    if (dist < min_dist) {
      min_dist = dist;
      nearest_idx = i;
    }
  }

  global_plan_idx_ = nearest_idx;
  return nearest_idx;
}

geometry_msgs::msg::PoseStamped SimplePursuitController::findLookaheadPoint(
  const geometry_msgs::msg::PoseStamped & robot_pose,
  size_t nearest_idx)
{
  for (size_t i = nearest_idx; i < global_plan_.poses.size(); ++i) {
    double dx = global_plan_.poses[i].pose.position.x - robot_pose.pose.position.x;
    double dy = global_plan_.poses[i].pose.position.y - robot_pose.pose.position.y;
    double dist = std::sqrt(dx * dx + dy * dy);

    if (dist >= lookahead_dist_) {
      return global_plan_.poses[i];
    }
  }

  return global_plan_.poses.back();
}

}  // namespace simple_pursuit_controller

PLUGINLIB_EXPORT_CLASS(simple_pursuit_controller::SimplePursuitController, nav2_core::Controller)