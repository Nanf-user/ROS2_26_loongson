#!/usr/bin/env python3
"""
路径走廊障碍物检测节点

订阅:
  /plan   (nav_msgs/Path)        — Nav2 全局路径
  /scan   (sensor_msgs/LaserScan) — 激光雷达

原理:
  将激光点 TF 变换到 map 坐标系后，
  检查是否有激光点落在全局路径的窄走廊内，
  赛道边界因为在走廊之外所以不会触发。
"""

import math
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy, DurabilityPolicy
from sensor_msgs.msg import LaserScan
from nav_msgs.msg import Path
from geometry_msgs.msg import PointStamped

import tf2_ros
import tf2_geometry_msgs          # 提供 do_transform_point


class PathObstacleDetector(Node):

    # ============================================================== #
    #                          初始化                                 #
    # ============================================================== #
    def __init__(self):
        super().__init__('path_obstacle_detector')

        # ---- 声明 & 读取参数 ----
        self.declare_parameter('corridor_half_width', 0.35)
        self.declare_parameter('lookahead_dist', 1.0)
        self.declare_parameter('min_valid_range', 0.15)
        self.declare_parameter('path_topic', '/plan')
        self.declare_parameter('scan_topic', '/scan')

        self.corridor_hw  = self.get_parameter('corridor_half_width').value
        self.lookahead    = self.get_parameter('lookahead_dist').value
        self.min_range    = self.get_parameter('min_valid_range').value

        # ---- TF2 ----
        self.tf_buffer   = tf2_ros.Buffer()
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)

        # ---- 状态 ----
        self.path_xy: list[tuple[float, float]] = []   # 路径点 (map)
        self.obstacle_active = False

        # ---- 订阅全局路径 ----
        #   Nav2 planner 使用 TRANSIENT_LOCAL 持久化
        path_qos = QoSProfile(
            depth=1,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.VOLATILE,
            history=HistoryPolicy.KEEP_LAST,
        )
        self.create_subscription(
            Path,
            self.get_parameter('path_topic').value,
            self._on_path,
            path_qos,
        )

        # ---- 订阅激光雷达 ----
        scan_qos = QoSProfile(
            depth=5,
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
        )
        self.create_subscription(
            LaserScan,
            self.get_parameter('scan_topic').value,
            self._on_scan,
            scan_qos,
        )

        self.get_logger().info(
            f'路径障碍物检测已启动 '
            f'[走廊半宽={self.corridor_hw} m, '
            f'前向距离={self.lookahead} m]'
        )

    # ============================================================== #
    #                     全局路径回调                                 #
    # ============================================================== #
    def _on_path(self, msg: Path):
        self.path_xy = [
            (p.pose.position.x, p.pose.position.y) for p in msg.poses
        ]
        self.get_logger().info(f'全局路径已更新 ({len(self.path_xy)} 个点)')

    # ============================================================== #
    #                     激光雷达回调                                 #
    # ============================================================== #
    def _on_scan(self, msg: LaserScan):

        path = self.path_xy
        if len(path) < 2:
            return                               # 还没有路径，跳过

        # ---------- 1. 获取 laser_frame → map 的 TF ----------
        try:
            tf = self.tf_buffer.lookup_transform(
                'map',                           # target
                msg.header.frame_id,             # source (如 laser_frame / base_scan)
                rclpy.time.Time(),               # 最新可用
                timeout=rclpy.duration.Duration(seconds=0.05),
            )
        except Exception as e:
            self.get_logger().warn(f'TF 查找失败: {e}', throttle_duration_sec=3.0)
            return

        # 激光原点在 map 中的坐标 ≈ 机器人位置
        rx = tf.transform.translation.x
        ry = tf.transform.translation.y

        # ---------- 2. 找路径上离机器人最近的锚点 ----------
        best_i = 0
        best_d = float('inf')
        for i, (px, py) in enumerate(path):
            d = math.hypot(rx - px, ry - py)
            if d < best_d:
                best_d, best_i = d, i

        # ---------- 3. 截取前方 lookahead 距离的路径子集 ----------
        sub: list[tuple[float, float]] = [path[best_i]]
        acc = 0.0
        for i in range(best_i, len(path) - 1):
            acc += math.hypot(path[i + 1][0] - path[i][0],
                              path[i + 1][1] - path[i][1])
            sub.append(path[i + 1])
            if acc >= self.lookahead:
                break

        if len(sub) < 2:
            return

        # ---------- 4. 遍历每一束激光 ----------
        hit = False
        closest = float('inf')

        for idx, r in enumerate(msg.ranges):
            # 过滤无效值
            if r < self.min_range or r > msg.range_max:
                continue
            if math.isinf(r) or math.isnan(r):
                continue

            # 构造 laser_frame 下的点
            angle = msg.angle_min + idx * msg.angle_increment
            pt = PointStamped()
            pt.header = msg.header
            pt.point.x = r * math.cos(angle)
            pt.point.y = r * math.sin(angle)
            pt.point.z = 0.0

            # 变换到 map
            try:
                mp = tf2_geometry_msgs.do_transform_point(pt, tf)
            except Exception:
                continue

            mx, my = mp.point.x, mp.point.y

            # 检查是否落在路径走廊内
            for j in range(len(sub) - 1):
                d = self._pt_seg_dist(
                    mx, my,
                    sub[j][0], sub[j][1],
                    sub[j + 1][0], sub[j + 1][1],
                )
                if d <= self.corridor_hw:
                    dist_to_robot = math.hypot(mx - rx, my - ry)
                    closest = min(closest, dist_to_robot)
                    hit = True
                    break                      # 这束光已命中，无需继续

        # ---------- 5. 输出结果 ----------
        if hit:
            if not self.obstacle_active:
                self.get_logger().warn(
                    f'⚠  前方路径上有障碍物！最近距离: {closest:.2f} m'
                )
                self.obstacle_active = True
        else:
            if self.obstacle_active:
                self.get_logger().info('路径障碍物已清除。')
                self.obstacle_active = False

    # ============================================================== #
    #                       工具函数                                   #
    # ============================================================== #
    @staticmethod
    def _pt_seg_dist(
        px: float, py: float,
        ax: float, ay: float,
        bx: float, by: float,
    ) -> float:
        """点 (px,py) 到线段 A→B 的最短距离"""
        dx, dy = bx - ax, by - ay
        len_sq = dx * dx + dy * dy
        if len_sq < 1e-12:                     # 线段退化为点
            return math.hypot(px - ax, py - ay)
        t = ((px - ax) * dx + (py - ay) * dy) / len_sq
        t = max(0.0, min(1.0, t))
        return math.hypot(px - (ax + t * dx), py - (ay + t * dy))


# ================================================================== #
#                              main                                  #
# ================================================================== #
def main(args=None):
    rclpy.init(args=args)
    node = PathObstacleDetector()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()