#!/usr/bin/env python3
import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource



def generate_launch_description():
    # RViZ2 settings
    rviz2_config = os.path.join(
        get_package_share_directory('e06_map_server'),
        'rviz',
        'map_server.rviz'
    )

    rviz2_node = Node(
        package='rviz2',
        executable='rviz2',
        name='map_server_rviz2',
        arguments=['-d',rviz2_config],
        output='screen'
    )

    #Include map_server launch file
    map_server_node = IncludeLaunchDescription(
        launch_description_source=PythonLaunchDescriptionSource([
            get_package_share_directory('e06_map_server'),
            '/launch/launch.py'
        ])
    )

    # 使用TimerAction实现延时启动
    delayed_rviz2_node = TimerAction(
        period=1.0,  # 延时1秒
        actions=[rviz2_node]
    )

    # Define LaunchDescription variable
    ld = LaunchDescription()

    ld.add_action(map_server_node)
    ld.add_action(delayed_rviz2_node)

    return ld