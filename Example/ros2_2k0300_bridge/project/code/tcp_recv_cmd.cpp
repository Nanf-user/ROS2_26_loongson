#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>      // 新增：用于 waitpid

#include "zf_common_headfile.h"    // 提供 system_delay_ms, tcp_init, tcp_client_read_data
#include "tcp_recv_cmd.h"
#include "get_config.h"

static int tcp_cmd_socket = -1;
static pid_t alarm_pid = -1;        // 播放子进程的 PID

// 启动循环播放 alarm.wav（后台运行）
void start_play_alarm(void)
{
    // 如果已有播放进程且仍在运行，则不重复启动
    if (alarm_pid > 0) {
        if (kill(alarm_pid, 0) == 0) {   // 进程存在
            return;
        } else {
            // 进程已退出，清理僵尸
            waitpid(alarm_pid, NULL, WNOHANG);
            alarm_pid = -1;
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed for alarm player\n");
        return;
    }

    if (pid == 0) {
        // 子进程：循环播放 alarm.wav
        // 使用 /bin/sh 执行 while 循环，确保 aplay 结束后重新播放
        execlp("/bin/sh", "sh", "-c", "while true; do aplay alarm.wav 2>/dev/null; done", NULL);
        // 如果 execlp 失败，退出子进程
        perror("execlp");
        exit(1);
    } else {
        // 父进程记录 PID
        alarm_pid = pid;
        printf("Alarm playback started (PID: %d)\n", pid);
    }
}

// 停止播放
void stop_play_alarm(void)
{
    if (alarm_pid > 0) {
        if (kill(alarm_pid, SIGTERM) == 0) {
            waitpid(alarm_pid, NULL, 0);   // 回收子进程
            printf("Alarm playback stopped (PID: %d)\n", alarm_pid);
        } else {
            // 可能已退出，回收僵尸
            waitpid(alarm_pid, NULL, WNOHANG);
        }
        alarm_pid = -1;
    }
}

// 关闭 TCP 连接时，同时停止播放
void close_tcp_recv_cmd(void)
{
    stop_play_alarm();                // 停止播放
    if (tcp_cmd_socket > 0) {
        close(tcp_cmd_socket);
        tcp_cmd_socket = -1;
    }
}

void tcp_recv_cmd_thd_entry(void)
{
    struct sched_param param;
    param.sched_priority = 6;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    while (1) {
        // ---------- 外循环：建立TCP连接 ----------
        do {
            tcp_cmd_socket = tcp_init(SERVER_IP, RECV_CMD_PORT);
            if (tcp_cmd_socket < 0) {
                printf("tcp_cmd_socket connect error, retry in 1s\n");
                system_delay_ms(1000);
            } else {
                printf("tcp_cmd_socket connected OK\n");
            }
        } while (tcp_cmd_socket <= 0);

        // ---------- 内循环：接收数据并处理 ----------
        while (1) {
            char recv_buf[64] = {0};
            int len = tcp_client_read_data(tcp_cmd_socket, (uint8_t *)recv_buf, sizeof(recv_buf) - 1);
            
            if (len <= 0) {
                printf("tcp_cmd_socket read error or closed, reconnecting...\n");
                stop_play_alarm();          // 断开时停止播放
                close(tcp_cmd_socket);
                tcp_cmd_socket = -1;
                break;                     // 跳出内循环，外循环重连
            }

            // 逐字节检查命令
            for (int i = 0; i < len; i++) {
                if (recv_buf[i] == '1') {
                    start_play_alarm();    // 收到 '1' 开始播放（已播放则忽略）
                } else if (recv_buf[i] == '2') {
                    stop_play_alarm();     // 收到 '2' 停止播放
                }
            }
        }
    }
}