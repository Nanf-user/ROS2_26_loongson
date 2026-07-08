#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include <pthread.h>
#include "zf_common_headfile.h"   // 提供 system_delay_ms, tcp_init, tcp_client_read_data

#include "tcp_camera.h"

using namespace cv;
using namespace std;

#define SERVER_IP   "10.91.26.9"   // 上位机IP（根据实际修改）
#define CAMERA_PORT 8893           // 修改后的端口

static int tcp_camera_socket = -1;

// 线程入口函数（无参无返回值，供 ThreadWrapper 调用）
void tcp_camera_thd_entry(void)
{
    // 设置线程优先级（可选，与原有代码一致）
    struct sched_param param;
    param.sched_priority = 5;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    // 打开摄像头（仅一次，避免反复初始化）
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        printf("Error: Cannot open camera.\n");
        return;
    }

    while (1) {
        // ---------- 外循环：建立TCP连接 ----------
        do {
            tcp_camera_socket = tcp_init(SERVER_IP, CAMERA_PORT);
            if (tcp_camera_socket < 0) {
                printf("tcp_camera_socket connect error, retry in 1s\n");
                system_delay_ms(1000);
            } else {
                printf("tcp_camera_socket connected OK\n");
            }
        } while (tcp_camera_socket <= 0);

        // ---------- 内循环：接收命令并处理 ----------
        while (1) {
            char recv_buf[64] = {0};
            int len = tcp_client_read_data(tcp_camera_socket, (uint8_t *)recv_buf, sizeof(recv_buf) - 1);

            if (len <= 0) {
                printf("tcp_camera_socket read error or closed, reconnecting...\n");
                close(tcp_camera_socket);
                tcp_camera_socket = -1;
                break;      // 跳出内循环，重连
            }

            // 逐字节处理命令
            for (int i = 0; i < len; i++) {
                if (recv_buf[i] == 'a') {
                    // 拍照
                    Mat frame;
                    cap >> frame;
                    if (frame.empty()) {
                        printf("Error: Captured frame is empty.\n");
                        continue;
                    }

                    // 编码为 JPEG
                    vector<uchar> buf;
                    vector<int> params = {IMWRITE_JPEG_QUALITY, 90};
                    if (!imencode(".jpg", frame, buf, params)) {
                        printf("Error: imencode failed.\n");
                        continue;
                    }

                    uint32_t size = buf.size();
                    uint32_t net_size = htonl(size);

                    // 发送长度（4字节）
                    if (send(tcp_camera_socket, &net_size, 4, 0) != 4) {
                        perror("send size");
                        close(tcp_camera_socket);
                        tcp_camera_socket = -1;
                        break;
                    }
                    // 发送图像数据
                    if (send(tcp_camera_socket, buf.data(), size, 0) != (ssize_t)size) {
                        perror("send data");
                        close(tcp_camera_socket);
                        tcp_camera_socket = -1;
                        break;
                    }
                    printf("Image sent, size: %u bytes\n", size);
                } else {
                    printf("Received unknown command: %c\n", recv_buf[i]);
                }
            }

            // 若因发送错误导致 socket 被关闭，则退出内循环触发重连
            if (tcp_camera_socket <= 0) {
                break;
            }
        }
    }
}