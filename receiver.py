#!/usr/bin/env python3
import socket
import struct
import cv2
import numpy as np
import datetime
import os

HOST = '0.0.0.0'
PORT = 8893
SAVE_DIR = "captures"          # 保存图像的目录

def recv_all(sock, n):
    data = b''
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            break
        data += chunk
    return data

def main():
    # 创建保存目录（如果不存在）
    if not os.path.exists(SAVE_DIR):
        os.makedirs(SAVE_DIR)
        print(f"创建目录: {SAVE_DIR}")

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(1)
    print(f"Server listening on {PORT}, waiting for board...")

    conn, addr = server.accept()
    print(f"Connected by {addr}")

    try:
        while True:
            cmd = input("Enter command ('a' to capture, 'q' to quit): ").strip().lower()
            if cmd == 'q':
                break
            elif cmd == 'a':
                conn.sendall(b'a')
                print("Sent 'a'")

                len_data = recv_all(conn, 4)
                if not len_data:
                    print("Connection closed by peer.")
                    break
                img_len = struct.unpack('>I', len_data)[0]
                print(f"Expecting image of {img_len} bytes")

                img_data = recv_all(conn, img_len)
                if len(img_data) != img_len:
                    print("Incomplete image data.")
                    break

                np_arr = np.frombuffer(img_data, dtype=np.uint8)
                img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
                if img is not None:
                    # 生成时间戳文件名（精确到毫秒）
                    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")[:-3]  # 保留3位毫秒
                    filename = os.path.join(SAVE_DIR, f"{timestamp}.jpg")
                    cv2.imwrite(filename, img)
                    print(f"Image saved as: {filename}")

                    # 显示图像
                    cv2.imshow("Captured Image", img)
                    cv2.waitKey(0)
                    cv2.destroyAllWindows()
                else:
                    print("Failed to decode image")
            else:
                print("Unknown command, use 'a' or 'q'")
    except KeyboardInterrupt:
        print("\nInterrupted")
    finally:
        conn.close()
        server.close()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()