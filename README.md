# ROS2 Humble — LS2K0300 微型自主小车系统

基于 **ROS2 Humble** 与 **龙芯 LS2K0300** 嵌入式开发板的微型自主小车系统，实现传感器数据采集、SLAM 建图、路径规划与自主导航。

## 系统架构

```
┌──────────────────────────────────────────────────┐
│              Ubuntu PC (上位机 / ROS2)              │
│                                                    │
│  ┌──────────┐ ┌───────────┐ ┌──────────────────┐  │
│  │  LiDAR   │ │  Odom/IMU │ │  cmd_vel 控制指令  │  │
│  │ TCP 接收  │ │  TCP 接收  │ │   TCP 发送        │  │
│  └────┬─────┘ └─────┬─────┘ └────────┬─────────┘  │
│       │              │               │             │
│       ▼              ▼               ▼             │
│  ┌─────────────────────────────────────────────┐  │
│  │          ROS2 导航栈 (Nav2)                   │  │
│  │  SLAM → 地图服务器 → 定位 → 路径规划 → 控制    │  │
│  └─────────────────────────────────────────────┘  │
│                                                    │
│  ┌──────────┐ ┌──────────┐ ┌──────────────────┐  │
│  │  相机     │ │  屏幕     │ │  障碍物检测       │  │
│  │  接收     │ │  指令发送  │ │  路径阻塞检测     │  │
│  └──────────┘ └──────────┘ └──────────────────┘  │
└────────────┬──────────────┬──────────────┬────────┘
             │ TCP (WiFi/以太网)              │
             ▼                               ▼
┌──────────────────────────────────────────────────┐
│         LS2K0300 嵌入式开发板 (下位机)              │
│                                                    │
│  ┌──────────┐ ┌──────────┐ ┌──────────────────┐  │
│  │ 10ms 定时 │ │ IMU 采集  │ │  编码器采集       │  │
│  │ 器回调    │ │ (六轴)    │ │  (里程计)         │  │
│  └──────────┘ └──────────┘ └──────────────────┘  │
│                                                    │
│  ┌──────────┐ ┌──────────┐ ┌──────────────────┐  │
│  │ PID 控制  │ │ PWM 电机  │ │  TCP 传感器发送   │  │
│  │ 速度闭环  │ │ 舵机控制  │ │  (IMU+编码器)     │  │
│  └──────────┘ └──────────┘ └──────────────────┘  │
│                                                    │
│  ┌──────────┐ ┌──────────┐ ┌──────────────────┐  │
│  │ 雷达串口  │ │ 相机采集  │ │  TCP 控制指令接收  │  │
│  │ TCP 发送  │ │ JPEG 编码 │ │  (cmd_vel)       │  │
│  └──────────┘ └──────────┘ └──────────────────┘  │
└──────────────────────────────────────────────────┘
```

## 目录结构

```
ROS2_Humble_LS2K0300/
├── Example/
│   ├── ros2_2k0300_bridge/        # LS2K0300 嵌入式桥接程序
│   │   ├── project/
│   │   │   ├── user/              # 主程序入口 + CMakeLists.txt + 交叉编译配置
│   │   │   │   ├── main.cpp       # 主控制循环（多线程）
│   │   │   │   ├── CMakeLists.txt # 构建配置
│   │   │   │   ├── cross.cmake    # LoongArch 交叉编译工具链
│   │   │   │   └── build.sh       # 编译 & scp 部署脚本
│   │   │   ├── code/              # 功能模块实现
│   │   │   │   ├── get_imu.cpp/h          # IMU 六轴数据采集
│   │   │   │   ├── get_encoder.cpp/h      # 编码器里程计采集
│   │   │   │   ├── get_radar.cpp/h        # 雷达数据协议解析
│   │   │   │   ├── motor_control.cpp/h    # PWM 电机 & 舵机控制
│   │   │   │   ├── user_pid.cpp/h         # PID 增量控制器
│   │   │   │   ├── tcp_send_sensor.cpp/h  # TCP 发送传感器数据
│   │   │   │   ├── tcp_send_ladar.cpp/h   # TCP 发送雷达数据
│   │   │   │   ├── tcp_recv_control.cpp/h # TCP 接收 cmd_vel 指令
│   │   │   │   ├── tcp_recv_cmd.cpp/h     # TCP 接收通用命令
│   │   │   │   ├── tcp_recv_display.cpp/h # TCP 接收屏幕显示指令
│   │   │   │   ├── tcp_camera.cpp/h       # TCP 相机采集与传输
│   │   │   │   ├── get_config.cpp/h       # 配置读取
│   │   │   │   ├── radar_data_protocol.cpp/h # 雷达协议处理
│   │   │   │   ├── class_posix_pit.cpp/h  # POSIX 定时器封装
│   │   │   │   └── class_thread_wrapper.h # 线程封装
│   │   │   └── out/               # 编译输出目录
│   │   ├── libraries/             # 逐飞科技 (SeekFree) 底层库
│   │   │   ├── zf_common/         # 通用定义与工具函数
│   │   │   ├── zf_components/     # 组件接口（IPS200 屏幕助手）
│   │   │   ├── zf_device/         # 设备驱动（IMU660RA/B、IMU963RA、IPS200、UVC）
│   │   │   └── zf_driver/         # 底层驱动（ADC、GPIO、PWM、PIT、Encoder、UDP/TCP）
│   │   ├── client.cpp             # TCP 客户端示例（x86）
│   │   ├── server.cpp             # TCP 服务端示例（x86）
│   │   └── ubuntu_server          # 编译好的 x86 服务端可执行文件
│   │
│   └── ROS2_Humble_ws/            # Ubuntu ROS2 工作空间
│       ├── src/
│       │   ├── e01_topic_pub/     # ROS2 话题发布示例 (C++)
│       │   ├── e01_topic_sub/     # ROS2 话题订阅示例 (C++)
│       │   ├── e02_laser_tcp_recv/        # LD19 激光雷达 TCP 接收驱动
│       │   ├── e02_odom_and_imu_tcp_recv/ # 里程计 & IMU TCP 接收节点
│       │   ├── e03_cmd_vel_tcp_send/      # cmd_vel 控制指令 TCP 发送节点
│       │   ├── e04_mini_car_description/  # 小车 URDF 模型描述
│       │   ├── e05_cartographer_mapping/  # Cartographer SLAM 建图
│       │   ├── e05_slam_toolbox_mapping/  # SLAM Toolbox 建图
│       │   ├── e06_map_server/            # 地图服务器（加载/保存）
│       │   ├── e07_nav2_bringup/          # Nav2 导航集成
│       │   │   ├── launch/        # 多种 launch 文件组合
│       │   │   ├── params/        # AMCL/DWB/TEB/Homing 参数
│       │   │   ├── bts/           # 行为树 XML 配置
│       │   │   ├── maps/          # 预建地图（PGM+YAML）
│       │   │   └── rviz/          # RVIZ2 配置文件
│       │   ├── simple_pursuit_controller/  # 纯追踪控制器插件
│       │   ├── path_blocked_tcp_bridge/    # 路径阻塞检测 TCP 桥接
│       │   └── path_obstacle_detector/     # 路径障碍物检测
│       ├── saved_images/          # 保存的导航截图
│       ├── build/                 # 编译构建目录
│       ├── install/               # 安装目录
│       └── log/                   # 日志目录
│
├── tcp/                           # 独立 TCP 相机传输示例
│   ├── user/                      # 源码 + CMakeLists + 交叉编译配置
│   └── out/                       # 编译输出
│
├── start1.sh                      # 一键启动脚本：传感器 + SLAM Toolbox + 键盘遥控
├── start2.sh                      # 一键启动脚本：传感器 + Nav2 定位 + DWB 控制器
├── start3.sh                      # 一键启动脚本：传感器 + Nav2 定位 + DWB + 路径阻塞检测
├── receiver.py                    # TCP 图像接收器（端口 8893，接收开发板相机图像）
├── tcp_cmd_server.py              # TCP 命令服务器（端口 8892，发送指令给开发板）
├── tcp_tsts.py                    # TCP 屏幕测试服务器（端口 8891，发送屏幕切换指令）
├── alarm.py                       # 警报音频生成器
├── alarm.wav                      # 生成的警报音频文件
├── alarm.zip                      # 警报音频压缩包
├── captures/                      # 相机抓拍保存目录
└── 【文档】说明书 芯片手册等/       # 芯片手册与参考文档
```

## 硬件平台

| 组件 | 型号 |
|------|------|
| 嵌入式开发板 | **龙芯 LS2K0300** (LoongArch 架构) |
| 上位机 | Ubuntu 22.04 + ROS2 Humble |
| 激光雷达 | LDROBOT LD19 (通过串口连接开发板) |
| IMU | 逐飞 IMU660RA / IMU660RB / IMU963RA |
| 电机驱动 | PWM 直流电机 + 舵机 |
| 编码器 | 增量式编码器（里程计） |
| 相机 | USB UVC 摄像头 |
| 显示屏 | IPS200 (SPI 接口) |
| 通信方式 | TCP/IP over WiFi / 以太网 |

## 通信端口分配

| 端口号 | 方向 | 用途 |
|--------|------|------|
| **8891** | PC → 开发板 | 屏幕显示指令（切换显示页面） |
| **8892** | PC → 开发板 | 通用命令（拍照等） |
| **8893** | 开发板 → PC | 相机图像数据传输 |
| 自定义 | 开发板 → PC | 雷达数据 TCP 透传 |
| 自定义 | 开发板 → PC | IMU + 里程计传感器数据 |
| 自定义 | PC → 开发板 | cmd_vel 控制指令 |

## 快速开始

### 1. 克隆项目

```bash
git clone <repo-url> ~/ROS2_Humble_LS2K0300
```

### 2. 编译 LS2K0300 嵌入式程序（交叉编译）

确保已安装 LoongArch 交叉编译工具链：

```bash
# 工具链路径（参考 cross.cmake 配置）
TOOLCHAIN_DIR="/opt/ls_2k0300_env/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.6"
```

编译并部署到开发板：

```bash
cd ~/ROS2_Humble_LS2K0300/Example/ros2_2k0300_bridge/project/user
bash build.sh
```

`build.sh` 会自动执行 `cmake` → `make` → `scp` 到开发板 (`root@10.91.26.227:/home/root/`)。

**注意**：编译前请根据实际网络环境修改 `cross.cmake` 中的工具链路径和 `build.sh` 中的开发板 IP 地址。

### 3. 编译 ROS2 工作空间（上位机）

```bash
cd ~/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws
colcon build --symlink-install
source install/setup.bash
```

### 4. 启动系统

提供了三种启动方式，根据需求选择：

#### 方式一：SLAM 建图 + 键盘遥控（建图模式）

```bash
cd ~/ROS2_Humble_LS2K0300
bash start1.sh
```

启动顺序：雷达 → 里程计/IMU → cmd_vel 发送 → URDF 模型 → RVIZ → SLAM Toolbox → 键盘遥控

#### 方式二：Nav2 定位 + DWB 路径跟踪

```bash
cd ~/ROS2_Humble_LS2K0300
bash start2.sh
```

启动顺序：传感器节点 → RVIZ → AMCL 定位 → DWB 局部路径控制器

#### 方式三：完整导航 + 障碍物/阻塞检测

```bash
cd ~/ROS2_Humble_LS2K0300
bash start3.sh
```

在方式二的基础上增加了 `path_blocked_tcp_bridge` 路径阻塞检测。

### 5. 手动启动单个组件

```bash
source ~/ROS2_Humble_LS2K0300/Example/ROS2_Humble_ws/install/setup.bash

# 启动激光雷达
ros2 launch e02_laser_tcp_recv ld19.launch.py

# 启动里程计和IMU
ros2 launch e02_odom_and_imu_tcp_recv launch.py

# 启动cmd_vel发送
ros2 launch e03_cmd_vel_tcp_send launch.py

# 启动URDF模型
ros2 launch e04_mini_car_description launch.py

# 启动SLAM Toolbox建图
ros2 launch e05_slam_toolbox_mapping slam_toolbox_launch.py

# 启动Cartographer建图
ros2 launch e05_cartographer_mapping cartographer_launch.py

# 启动地图服务器
ros2 launch e06_map_server launch.py

# 启动Nav2 AMCL定位
ros2 launch e07_nav2_bringup nav2_localization_launch.py

# 启动Nav2 DWB控制器
ros2 launch e07_nav2_bringup nav2_dwb_launch.py

# 启动RVIZ可视化
ros2 launch e07_nav2_bringup rviz_launch.py

# 键盘遥控
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

### 6. 辅助工具

```bash
# 启动图像接收服务器（接收开发板相机图像）
python3 receiver.py

# 启动命令发送服务器（向开发板发送命令）
python3 tcp_cmd_server.py

# 启动屏幕切换测试服务器
python3 tcp_tsts.py

# 生成警报音频
python3 alarm.py
```

## 数据流说明

### 上行数据（开发板 → Ubuntu PC）

```
┌─────────────────┐     TCP      ┌──────────────────────────────┐
│  LS2K0300       │ ──────────→  │  ROS2 节点                    │
│                 │              │                               │
│  雷达 (LD19)     │ ──雷达数据──→│  e02_laser_tcp_recv           │
│  编码器 (Odom)   │ ──里程计───→│  e02_odom_and_imu_tcp_recv   │
│  IMU (六轴)      │ ──IMU数据──→│  (同上)                       │
│  相机 (UVC)      │ ──JPEG图片─→│  receiver.py (端口 8893)      │
└─────────────────┘              └──────────────────────────────┘
```

### 下行数据（Ubuntu PC → 开发板）

```
┌──────────────────────────────┐     TCP      ┌─────────────────┐
│  ROS2 节点                    │ ──────────→  │  LS2K0300       │
│                               │              │                 │
│  cmd_vel → e03_cmd_vel_tcp   │ ──速度指令──→│  PID → PWM 电机 │
│  tcp_cmd_server.py (8892)    │ ──拍照指令──→│  拍照 → JPEG    │
│  tcp_tsts.py (8891)          │ ──显示指令──→│  IPS200 屏幕    │
└──────────────────────────────┘              └─────────────────┘
```

## 控制逻辑（下位机）

LS2K0300 开发板运行多线程控制程序 (`main.cpp`)：

| 线程 | 功能 | 说明 |
|------|------|------|
| 10ms 定时器 | IMU + 编码器采集 + PID 控制 + PWM 输出 | 核心控制循环 |
| Thread 0 | 传感器数据 TCP 发送 (IMU + 编码器) | 10ms 周期发送 |
| Thread 1 | 雷达数据 TCP 发送 | 串口接收雷达 → TCP 透传 |
| Thread 2 | cmd_vel TCP 接收 | 接收 ROS2 速度指令，更新 PID 目标 |
| Thread 3 | 屏幕指令 TCP 接收 | 切换 IPS200 显示页面 |
| Thread 4 | 通用命令 TCP 接收 | 拍照等指令 |
| Thread 5 | 相机 TCP 发送 | 收到拍照指令后采集并 JPEG 编码发送 |

**PID 控制**：采用增量式 PID，左轮右轮独立控制，速度闭环。舵机通过 PWM 控制转向角度。

## ROS2 包列表

| 包名 | 类型 | 功能描述 |
|------|------|----------|
| `e01_topic_pub` | C++ | ROS2 话题发布示例 |
| `e01_topic_sub` | C++ | ROS2 话题订阅示例 |
| `e02_laser_tcp_recv` | C++ | LD19 激光雷达 TCP 接收驱动与可视化 |
| `e02_odom_and_imu_tcp_recv` | C++ | 里程计 / IMU TCP 接收与 TF 发布 |
| `e03_cmd_vel_tcp_send` | C++ | cmd_vel 速度指令 TCP 发送 |
| `e04_mini_car_description` | URDF | 小车 URDF 模型 + RVIZ 可视化 |
| `e05_cartographer_mapping` | C++ | Google Cartographer SLAM 建图 |
| `e05_slam_toolbox_mapping` | C++ | SLAM Toolbox 建图 |
| `e06_map_server` | C++ | 地图服务器（加载/保存 PGM 地图） |
| `e07_nav2_bringup` | C++ | Nav2 导航集成（AMCL + DWB + TEB + Homing） |
| `simple_pursuit_controller` | C++ | 纯追踪 (Pure Pursuit) 路径跟踪控制器 |
| `path_blocked_tcp_bridge` | Python | 路径阻塞状态 TCP 桥接到开发板 |
| `path_obstacle_detector` | Python | 基于激光雷达的路径障碍物检测 |

## 关键技术栈

- **ROS2 Humble** — 机器人操作系统
- **Nav2** — 导航框架 (AMCL 定位 + DWB/TEB 规划器 + 行为树)
- **SLAM Toolbox / Cartographer** — SLAM 建图
- **龙芯 LS2K0300** — LoongArch 架构嵌入式处理器
- **交叉编译** — `loongarch64-linux-gnu-gcc/g++` 工具链
- **OpenCV 4.10** — 图像编解码
- **逐飞科技 (SeekFree) 库** — 嵌入式底层驱动
- **CMake** — 跨平台构建系统
- **Python** — 辅助脚本与工具

## 开发环境

- **上位机**：Ubuntu 22.04 + ROS2 Humble + colcon
- **下位机交叉编译工具链**：`loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.6`
- **OpenCV**：4.10.0（交叉编译版本，位于 `/opt/ls_2k0300_env/opencv_4_10_build`）
- **VS Code**：已配置 ROS2 Humble 开发环境（`.vscode/settings.json`）

## 文档资源

- `【文档】说明书 芯片手册等/例程使用手册.pdf` — 例程使用手册
- `【文档】说明书 芯片手册等/虚拟机下载地址【已配置好】.txt` — 预配置虚拟机下载地址
- `Example/ROS2_Humble_ws/src/e02_laser_tcp_recv/README.md` — 激光雷达 SDK 说明（中英文）
- `Example/ROS2_Humble_ws/src/e03_cmd_vel_tcp_send/read.md` — cmd_vel 发送节点说明

## License

MIT License

---

**注意**：本项目针对特定硬件平台（龙芯 LS2K0300 + 逐飞外设 + LD19 雷达），更换硬件需要修改相应的驱动代码与设备配置。网络配置（IP 地址、端口号）请根据实际环境调整。
