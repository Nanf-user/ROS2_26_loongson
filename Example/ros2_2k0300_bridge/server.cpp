#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LISTEN_PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    // 1. 创建TCP socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("socket 创建失败");
        return -1;
    }

    // 端口复用，避免重启提示端口占用
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // 绑定本机所有网卡
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(LISTEN_PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind 绑定端口失败");
        close(server_sock);
        return -1;
    }

    // 开启监听，最大挂起连接数3
    if (listen(server_sock, 3) < 0)
    {
        perror("listen 监听失败");
        close(server_sock);
        return -1;
    }
    std::cout << "Ubuntu TCP服务端已启动，监听端口：" << LISTEN_PORT << std::endl;
    std::cout << "等待设备连接..." << std::endl;

    // 循环接收客户端连接
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0)
        {
            perror("accept 接收连接失败");
            continue;
        }

        // 打印客户端IP
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        std::cout << "\n新客户端接入：" << client_ip << std::endl;

        char recv_buf[BUFFER_SIZE] = {0};
        // 持续收发数据
        while (true)
        {
            memset(recv_buf, 0, BUFFER_SIZE);
            int recv_len = recv(client_sock, recv_buf, BUFFER_SIZE, 0);
            // recv_len<=0 代表客户端断开
            if (recv_len <= 0)
            {
                std::cout << "客户端 " << client_ip << " 断开连接" << std::endl;
                break;
            }
            std::cout << "收到客户端数据：" << recv_buf << std::endl;

            // 回发应答给客户端
            std::string reply = "Ubuntu服务端已收到：" + std::string(recv_buf);
            send(client_sock, reply.c_str(), reply.size(), 0);
        }
        close(client_sock);
    }

    close(server_sock);
    return 0;
}