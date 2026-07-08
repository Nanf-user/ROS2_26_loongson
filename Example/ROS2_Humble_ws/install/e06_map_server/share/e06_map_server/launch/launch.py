import os
import logging
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory



def generate_launch_description():
  # 获取当前功能包的共享目录
  pkg_share_dir = get_package_share_directory('e06_map_server')
  # 构建 map 文件的完整路径
  map_file = os.path.join(pkg_share_dir, 'map', 'zxy.yaml')
  if not os.path.exists(map_file):
      logging.error(f"地图文件 {map_file} 不存在，请检查路径。")
      return LaunchDescription([])
  
  map_server_node = Node(
      package='nav2_map_server',
      executable='map_server',
      name='map_server',
      output='screen',
      parameters=[{'use_sim_time': False},
                  {'yaml_filename':map_file}]
  )
  manager_mapper_node = Node(
    package='nav2_lifecycle_manager',
    executable='lifecycle_manager',
    name='lifecycle_manager_mapper',
    output='screen',
    parameters=[{'use_sim_time': False},
      {'autostart': True},
      {'node_names': ['map_server']}]
  )
  return LaunchDescription([map_server_node,manager_mapper_node])