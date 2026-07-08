#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 改成你的Ubuntu虚拟机IP
#define SERVER_IP "10.190.84.9"
#define SERVER_PORT 8080
#define BUF_LEN 1024

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket 创建失败");
        return -1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        perror("IP地址无效");
        close(sock);
        return -1;
    }

    // 连接Ubuntu服务端
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect fail");
        close(sock);
        return -1;
    }
    std::cout << "成功连接Ubuntu服务端" << std::endl;

    char buf[BUF_LEN] = {0};
    while (true)
    {
        std::cout << "输入发送消息，quit退出：";
        std::cin >> buf;
        std::string input(buf);
        if (input == "quit") break;

        send(sock, buf, strlen(buf), 0);
        memset(buf, 0, BUF_LEN);
        recv(sock, buf, BUF_LEN, 0);
        std::cout << "服务端应答：" << buf << std::endl;
    }

    close(sock);
    return 0;
}