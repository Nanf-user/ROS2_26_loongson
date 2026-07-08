import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_share = get_package_share_directory("e07_nav2_bringup")
    
    # 启动RViz
    rviz_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_share, 'launch', 'rviz_launch.py')
        )
    )
    
    # 启动定位
    loca_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_share, 'launch', 'local_launch.py')
        )
    )
    
    # 启动导航（添加2秒延时）
    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_share, 'launch', 'nav2_dwb_launch.py')
        )
    )
    
    # 使用TimerAction实现延时启动
    delayed_nav2_launch = TimerAction(
        period=1.0,  # 延时1秒
        actions=[nav2_launch]
    )


    # 使用TimerAction实现延时启动
    delayed_loca_launch = TimerAction(
        period=2.0,  # 延时2秒
        actions=[loca_launch]
    )
    
    return LaunchDescription([
        rviz_launch,
        delayed_loca_launch,
        delayed_nav2_launch  # 替换为延时启动动作
    ])