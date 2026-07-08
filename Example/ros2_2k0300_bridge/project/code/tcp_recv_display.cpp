#include "tcp_recv_display.h"
#include "radar_data_protocol.h"
#include "get_config.h"
#include "zf_common_headfile.h"

// 全局变量
screen_cmd_t tcp_screen_cmd;     // 临时接收缓冲区
screen_cmd_t g_screen_cmd;       // 全局生效的指令

static int tcp_client_screen_socket = -1;   // socket描述符

// 关闭连接函数（供 cleanup 调用）
void close_tcp_recv_screen(void)
{
    if (tcp_client_screen_socket > 0) {
        close(tcp_client_screen_socket);
        tcp_client_screen_socket = -1;
    }
}

// 线程主函数
void tcp_recv_screen_thd_entry(void)
{
    // 设置线程优先级
    struct sched_param param;
    param.sched_priority = 30;   // 比 TCP_RECV_CONTROL_THD_PRO 低一些
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    

    printf("tcp_recv_screen thread started...\r\n");
    // ips200_draw_image(0,0,emoji_1, 240, 320);
    // system_delay_ms(50);
    while(1)
    {
        // ----- 外循环：尝试连接服务器（阻塞直到连接成功）-----
        do {
            tcp_client_screen_socket = tcp_init(SERVER_IP, SCREEN_MIC_PORT);  // SCREEN_PORT 需定义
            if (tcp_client_screen_socket < 0) {
                system_delay_ms(1000);
                printf("tcp_client_screen_socket error, retrying...\r\n");
            } else {
                printf("tcp_client_screen_socket OK\r\n");
            }
        } while (tcp_client_screen_socket <= 0);

        // ----- 内循环：接收数据 -----
        while (1) {
            // 读取固定大小的结构体
            int str_len = tcp_client_read_data(tcp_client_screen_socket, 
                                               (uint8_t *)&tcp_screen_cmd, 
                                               sizeof(screen_cmd_t));
            if (str_len <= 0) {
                // 连接断开，清空临时变量，退出内循环重连
                tcp_screen_cmd.cmd = 0;
                tcp_screen_cmd.image_id = 0;
                printf("Screen control connection lost, reconnecting...\r\n");
                break;
            } else {
                // 成功接收数据，更新全局变量
                g_screen_cmd.cmd = tcp_screen_cmd.cmd;
                g_screen_cmd.image_id = tcp_screen_cmd.image_id;

                // 打印调试信息（可关闭）
                printf("Screen cmd: %d, image_id: %d\r\n", 
                       g_screen_cmd.cmd, g_screen_cmd.image_id);

                // ---- 立即执行屏幕显示（因为图片显示不要求硬实时） ----
                // 根据命令执行对应的图片显示
                if (g_screen_cmd.cmd == 0) {
                    // 显示默认背景
                    ips200_draw_image(0,0,emoji_1, 240,320);
                } else if (g_screen_cmd.cmd == 1) {
                    // 显示障碍物警告图片
                    ips200_draw_image(0,0,emoji_2, 240,320);
                } else if (g_screen_cmd.cmd == 2) {
                    // 显示笑脸或其他
                    ips200_draw_image(0,0,emoji_3, 240,320);
                } else {
                    // 未知命令，忽略或清屏
                }
            }
        }
    }
}