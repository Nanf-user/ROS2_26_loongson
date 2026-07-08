#ifndef NAV2_OBSTACLE_AHEAD_CONDITION_HPP_
#define NAV2_OBSTACLE_AHEAD_CONDITION_HPP_

#include <string>
#include <memory>
#include <mutex>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "behaviortree_cpp_v3/condition_node.h"

namespace nav2_behavior_tree
{

class ObstacleAheadCondition : public BT::ConditionNode
{
public:
  // v3 用 NodeConfiguration，不是 NodeConfig
  ObstacleAheadCondition(
    const std::string & condition_name,
    const BT::NodeConfiguration & conf);

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<double>(
        "distance_threshold", 1.0,
        "Obstacle detection distance threshold (meters)"),
      BT::InputPort<double>(
        "angle_range", 0.5,
        "Forward detection angle range (radians)"),
      BT::InputPort<std::string>(
        "scan_topic", std::string("/scan"),
        "Laser scan topic name"),
    };
  }

  BT::NodeStatus tick() override;
  // v3 中 ConditionNode::halt() 是 final，不能 override，删掉 halt

private:
  void laserCallback(sensor_msgs::msg::LaserScan::SharedPtr scan);

  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;

  std::mutex scan_mutex_;
  sensor_msgs::msg::LaserScan::SharedPtr latest_scan_;
};

}  // namespace nav2_behavior_tree

#endif  // NAV2_OBSTACLE_AHEAD_CONDITION_HPP_