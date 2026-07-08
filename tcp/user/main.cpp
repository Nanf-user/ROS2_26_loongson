#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>

using namespace cv;
using namespace std;

const char* SERVER_IP = "10.91.26.9"; // 修改为上位机IP
const int PORT = 8891;

// 安全接收指定字节数
bool recv_all(int sock, void* buffer, size_t n) {
    size_t total = 0;
    char* buf = (char*)buffer;
    while (total < n) {
        ssize_t received = recv(sock, buf + total, n - total, 0);
        if (received <= 0) {
            return false;
        }
        total += received;
    }
    return true;
}

int main() {
    // 打开摄像头
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: Cannot open camera." << endl;
        return -1;
    }

    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    // 连接服务器
    cout << "Connecting to " << SERVER_IP << ":" << PORT << " ..." << endl;
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }
    cout << "Connected to server." << endl;

    // 主循环：保持长连接
    while (true) {
        char cmd;
        // 接收命令，至少1字节
        ssize_t n = recv(sock, &cmd, 1, 0);
        if (n <= 0) {
            // 连接断开或错误
            cerr << "Connection closed or error, exiting." << endl;
            break;
        }

        if (cmd == 'a') {
            // 拍照
            Mat frame;
            cap >> frame;
            if (frame.empty()) {
                cerr << "Error: Captured frame is empty." << endl;
                continue;
            }

            // 编码为JPEG
            vector<uchar> buf;
            vector<int> params = {IMWRITE_JPEG_QUALITY, 90};
            if (!imencode(".jpg", frame, buf, params)) {
                cerr << "Error: imencode failed." << endl;
                continue;
            }

            uint32_t size = buf.size();
            uint32_t net_size = htonl(size);

            // 发送长度（4字节）
            if (send(sock, &net_size, 4, 0) != 4) {
                perror("send size");
                break;
            }
            // 发送图像数据
            if (send(sock, buf.data(), size, 0) != (ssize_t)size) {
                perror("send data");
                break;
            }
            cout << "Image sent, size: " << size << " bytes" << endl;
        } else {
            // 其他命令可以忽略或处理
            cout << "Received unknown command: " << cmd << endl;
        }
    }

    close(sock);
    cap.release();
    return 0;
}