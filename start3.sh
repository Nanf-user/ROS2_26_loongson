#!/bin/bash
# 工作空间路径
WS_PATH="$HOME/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws"

# 定义 source 命令，每个终端都要执行
SRC_CMD="cd $WS_PATH && source install/setup.bash"

# 进入工作空间
cd "$WS_PATH" || { echo "错误：工作空间目录不存在！"; exit 1; }
source install/setup.bash

echo "===== 启动第一组节点 ====="
# 1. 雷达
gnome-terminal -- bash -c "$SRC_CMD && ros2 launch e02_laser_tcp_recv ld19.launch.py; exec bash"
sleep 1
# 2. 里程计IMU
gnome-terminal -- bash -c "$SRC_CMD && ros2 launch e02_odom_and_imu_tcp_recv launch.py; exec bash"
sleep 1
# 3. 底盘指令发送
gnome-terminal -- bash -c "$SRC_CMD && ros2 launch e03_cmd_vel_tcp_send launch.py; exec bash"
sleep 1
# 4. URDF模型
gnome-terminal -- bash -c "$SRC_CMD && ros2 launch e04_mini_car_description launch.py; exec bash"
sleep 1

echo ""
echo "======================================================"
echo "前4个硬件节点已启动，请确认雷达/里程计数据正常！"
echo "按下【回车键】继续启动RVIZ、SLAM、键盘遥控..."
echo "======================================================"
read -r pause_input

echo "===== 启动第二组节点 ====="
# RVIZ
gnome-terminal -- bash -c "$SRC_CMD && ros2 launch e07_nav2_bringup rviz_launch.py; exec bash"
sleep 2
# SLAM建图
gnome-terminal -- bash -c "$SRC_CMD && ros2 launch e07_nav2_bringup local_launch.py; exec bash"
sleep 1
# 键盘遥控
gnome-terminal -- bash -c "$SRC_CMD && ros2 launch e07_nav2_bringup nav2_dwb_launch.py; exec bash"

gnome-terminal -- bash -c "$SRC_CMD && ros2 run path_blocked_tcp_bridge path_blocked_tcp_sender; exec bash"

echo -e "\n========== 全部节点启动完成 =========="
echo "如需一键关闭所有ROS进程，请按下回车键"
echo "======================================"
read -r exit_input

echo "正在终止所有ROS2进程..."
pkill -f ros2
sleep 0.5
echo "所有进程已关闭，脚本退出！"
