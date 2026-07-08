#!/usr/bin/env python3
import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

def generate_launch_description():
  # RViZ2 settings
  rviz2_config = os.path.join(
      get_package_share_directory('e02_odom_and_imu_tcp_recv'),
      'rviz',
      'odom_imu.rviz'
  )

  rviz2_node = Node(
      package='rviz2',
      executable='rviz2',
      name='odom_imu_launch',
      arguments=['-d',rviz2_config],
      output='screen'
  )

  #Include odom_imu launch file
  odom_imu_launch = IncludeLaunchDescription(
      launch_description_source=PythonLaunchDescriptionSource([
          get_package_share_directory('e02_odom_and_imu_tcp_recv'),
          '/launch/odom_imu_launch.py'
      ])
  )

  # Define LaunchDescription variable
  ld = LaunchDescription()

  ld.add_action(odom_imu_launch)
  ld.add_action(rviz2_node)

  return ld