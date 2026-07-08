# Install script for directory: /home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/src/e03_cmd_vel_tcp_send

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/install/e03_cmd_vel_tcp_send")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send/cmd_vel_tcp_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send/cmd_vel_tcp_node")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send/cmd_vel_tcp_node"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send" TYPE EXECUTABLE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/cmd_vel_tcp_node")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send/cmd_vel_tcp_node" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send/cmd_vel_tcp_node")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send/cmd_vel_tcp_node"
         OLD_RPATH "/opt/ros/humble/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/e03_cmd_vel_tcp_send/cmd_vel_tcp_node")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send/" TYPE DIRECTORY FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/src/e03_cmd_vel_tcp_send/launch")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/package_run_dependencies" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_index/share/ament_index/resource_index/package_run_dependencies/e03_cmd_vel_tcp_send")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/parent_prefix_path" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_index/share/ament_index/resource_index/parent_prefix_path/e03_cmd_vel_tcp_send")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send/environment" TYPE FILE FILES "/opt/ros/humble/share/ament_cmake_core/cmake/environment_hooks/environment/ament_prefix_path.sh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send/environment" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_environment_hooks/ament_prefix_path.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send/environment" TYPE FILE FILES "/opt/ros/humble/share/ament_cmake_core/cmake/environment_hooks/environment/path.sh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send/environment" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_environment_hooks/path.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_environment_hooks/local_setup.bash")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_environment_hooks/local_setup.sh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_environment_hooks/local_setup.zsh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_environment_hooks/local_setup.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_environment_hooks/package.dsv")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ament_index/resource_index/packages" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_index/share/ament_index/resource_index/packages/e03_cmd_vel_tcp_send")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send/cmake" TYPE FILE FILES
    "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_core/e03_cmd_vel_tcp_sendConfig.cmake"
    "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/ament_cmake_core/e03_cmd_vel_tcp_sendConfig-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/e03_cmd_vel_tcp_send" TYPE FILE FILES "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/src/e03_cmd_vel_tcp_send/package.xml")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/niuwa/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/build/e03_cmd_vel_tcp_send/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
