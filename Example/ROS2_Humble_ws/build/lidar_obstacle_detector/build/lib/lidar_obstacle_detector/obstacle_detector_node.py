#!/usr/bin/env python3
"""
激光雷达障碍物检测节点
- 订阅 /scan 话题 (sensor_msgs/LaserScan)
- 检测前方指定角度范围内的最近障碍物距离
- 小于安全距离时在终端打印警告
"""

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan
import math


class ObstacleDetectorNode(Node):
    def __init__(self):
        super().__init__('obstacle_detector')

        # ========== 参数声明 ==========
        # 安全距离（米）：低于此距离认为有障碍物
        self.declare_parameter('safe_distance', 0.3)
        # 前方检测角度范围（度）：以正前方为中心，左右各 half_angle 度
        self.declare_parameter('half_angle', 15.0)

        self.safe_distance = self.get_parameter('safe_distance').value
        self.half_angle = self.get_parameter('half_angle').value

        self.get_logger().info(
            f'障碍物检测节点已启动 '
            f'[安全距离: {self.safe_distance}m, '
            f'检测角度: ±{self.half_angle}°]'
        )

        # ========== 订阅激光雷达话题 ==========
        # QoS: 使用 BEST_EFFORT 匹配大多数雷达的默认发布策略
        from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy
        qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )
        self.subscriber = self.create_subscription(
            LaserScan,
            '/scan',
            self.scan_callback,
            qos
        )

        # 标记：避免每次扫描都重复打印
        self.obstacle_detected = False

    def scan_callback(self, msg: LaserScan):
        """
        回调函数：解析 LaserScan 数据，检测前方障碍物
        msg.angle_min   : 扫描起始角度 (rad)
        msg.angle_max   : 扫描结束角度 (rad)
        msg.angle_increment : 角度分辨率 (rad)
        msg.ranges      : 距离数组
        """
        # 计算正前方对应的索引
        # 0 弧度 = 正前方（大多数雷达的约定）
        num_readings = len(msg.ranges)

        # 将角度范围转换为索引范围
        half_angle_rad = math.radians(self.half_angle)

        # 找出落在 [-half_angle, +half_angle] 范围内的所有有效测量值
        min_distance = float('inf')
        closest_angle = 0.0

        for i, distance in enumerate(msg.ranges):
            angle = msg.angle_min + i * msg.angle_increment

            # 只看前方扇区
            if -half_angle_rad <= angle <= half_angle_rad:
                # 过滤无效值（inf, nan, 小于最小范围的值）
                if (msg.range_min <= distance <= msg.range_max
                        and not math.isinf(distance)
                        and not math.isnan(distance)):
                    if distance < min_distance:
                        min_distance = distance
                        closest_angle = angle

        # ========== 判断并打印 ==========
        if min_distance < self.safe_distance:
            if not self.obstacle_detected:
                self.get_logger().warn(
                    f'⚠ 前方有障碍物！'
                    f'距离: {min_distance:.2f}m, '
                    f'角度: {math.degrees(closest_angle):.1f}°'
                )
                self.obstacle_detected = True
        else:
            if self.obstacle_detected:
                self.get_logger().info('前方障碍物已清除，道路畅通。')
                self.obstacle_detected = False


def main(args=None):
    rclpy.init(args=args)
    node = ObstacleDetectorNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()