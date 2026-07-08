#!/usr/bin/env python3
import socket
import struct
import time

HOST = '0.0.0.0'          # 监听所有网络接口
PORT = 8891               # 与开发板 SCREEN_MIC_PORT 一致

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(1)
    print(f"Screen server listening on {PORT}, waiting for board to connect...")

    conn, addr = server.accept()
    print(f"Board connected from {addr}")

    try:
        cmd = 0
        while True:
            # 打包命令 (cmd, image_id) 两个字节
            packet = struct.pack('BB', cmd, 0)
            conn.sendall(packet)
            print(f"Sent: cmd={cmd}")
            time.sleep(5)
            cmd = (cmd + 1) % 4   # 0,1,2,3 循环
    except KeyboardInterrupt:
        print("\nStopped by user")
    finally:
        conn.close()
        server.close()

if __name__ == "__main__":
    main()