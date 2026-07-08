from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='e01_topic_sub',  # 功能包名称
            executable='listener',            # 可执行文件名称
            output='screen',                      # 输出到终端
            parameters=[                          # 可选：传递参数
                # {'param_name': 'param_value'}
            ]
        ),
    ])