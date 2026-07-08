import socket
import threading
import sys

def handle_client(client_socket, client_addr):
    """处理与客户端的通信：接收用户输入并发送"""
    print(f"[*] 客户端 {client_addr} 已连接")
    try:
        while True:
            # 获取用户输入
            msg = input(">>> 请输入要发送的消息 (输入 'exit' 退出): ")
            if msg.lower() in ('exit', 'quit'):
                print("[*] 退出程序")
                break
            # 发送数据（编码为字节）
            client_socket.sendall(msg.encode('utf-8'))
            print(f"[+] 已发送: {msg}")
    except (ConnectionResetError, BrokenPipeError):
        print("[!] 客户端连接已断开")
    except KeyboardInterrupt:
        print("\n[!] 用户中断")
    finally:
        client_socket.close()
        print("[*] 连接关闭，程序退出")
        sys.exit(0)

def start_server(host='0.0.0.0', port=8892):
    """启动 TCP 服务器，监听指定端口"""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((host, port))
    server.listen(1)   # 只允许一个连接（可按需调整）
    print(f"[*] TCP 服务器已启动，监听 {host}:{port}")
    print("[*] 等待下位机连接...")

    while True:
        client_socket, client_addr = server.accept()
        # 处理客户端（单线程模式，输入阻塞，适合手动发送）
        handle_client(client_socket, client_addr)

if __name__ == "__main__":
    start_server()