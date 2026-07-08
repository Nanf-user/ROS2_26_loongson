import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():

    slam_pkg = get_package_share_directory("e05_slam_toolbox_mapping")
    nav2_pkg = get_package_share_directory("e07_nav2_bringup")
    rviz_pkg = get_package_share_directory("e07_nav2_bringup")
    
    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(slam_pkg,'launch',
                                                    'slam_toolbox_launch.py'))
        )

    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(nav2_pkg,'launch', 
                                                    'navigation_launch.py'))
        )

    rviz_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(rviz_pkg,'launch', 
                                                    'rviz_launch.py'))
        )


    ld = LaunchDescription()
    ld.add_action(slam_launch)
    ld.add_action(nav2_launch)
    ld.add_action(rviz_launch)
    
    return ld