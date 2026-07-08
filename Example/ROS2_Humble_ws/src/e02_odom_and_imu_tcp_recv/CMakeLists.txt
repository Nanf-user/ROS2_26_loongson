cmake_minimum_required(VERSION 3.8)
project(e02_odom_and_imu_tcp_recv)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)




# 查找并引入 ament_cmake 包，ament_cmake 是 ROS 2 中用于构建和管理包的工具集，是必需的依赖
find_package(ament_cmake REQUIRED)
# 查找并引入 rclcpp 包，rclcpp 是 ROS 2 的 C++ 客户端库，用于实现 ROS 2 节点和通信功能，是必需的依赖
find_package(rclcpp REQUIRED)
# 查找并引入 sensor_msgs 包，sensor_msgs 包含了各种传感器消息类型的定义，如激光雷达、摄像头等消息，是必需的依赖
find_package(sensor_msgs REQUIRED)
# 查找并引入 nav_msgs 包，nav_msgs 包含了导航相关的消息类型定义，如里程计、路径等消息，是必需的依赖
find_package(nav_msgs REQUIRED)
# 查找并引入 tf2 包，tf2 是 ROS 2 中用于处理坐标系变换的库，是必需的依赖
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(odom_and_imu_tcp_recv src/main.cpp src/car_point.cpp)
ament_target_dependencies(odom_and_imu_tcp_recv rclcpp sensor_msgs nav_msgs tf2 tf2_ros geometry_msgs)

target_link_libraries(odom_and_imu_tcp_recv boost_system boost_thread)
ament_target_dependencies(odom_and_imu_tcp_recv rclcpp nav_msgs sensor_msgs tf2 tf2_ros)


install(TARGETS
  odom_and_imu_tcp_recv
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch rviz
  DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
