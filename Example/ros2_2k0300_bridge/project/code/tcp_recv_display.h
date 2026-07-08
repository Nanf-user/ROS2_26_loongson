#ifndef TCP_RECV_DISPLAY_H
#define TCP_RECV_DISPLAY_H

#include "zf_common_headfile.h"

// 屏幕控制命令结构体（与ROS端约定）
typedef struct __attribute__((packed)) {
    uint8_t  cmd;        // 命令：0=显示默认图片, 1=显示障碍物图片, 2=显示笑脸, 3=清屏等
    uint8_t  image_id;   // 图片ID（如果有多张图片）
    // 也可以加其他参数，但简化，只用一个ID
} screen_cmd_t;

// 全局变量声明（供其他文件使用）


// 线程入口函数
void tcp_recv_screen_thd_entry(void);

// 关闭连接函数（用于cleanup）
void close_tcp_recv_screen(void);

#endif