import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/install/lidar_obstacle_detector'
