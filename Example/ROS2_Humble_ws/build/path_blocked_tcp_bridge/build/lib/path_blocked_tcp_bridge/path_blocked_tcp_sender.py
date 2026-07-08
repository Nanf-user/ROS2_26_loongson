#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool
import socket
import struct
import threading
import time
import os
import cv2
import numpy as np

class PathBlockedTcpServer(Node):
    def __init__(self):
        super().__init__('path_blocked_tcp_server')
        
        # 声明并获取参数
        self.declare_parameter('topic', '/path_blocked')
        topic_name = self.get_parameter('topic').value
        
        # 线程锁，保护 socket 对象
        self.lock = threading.Lock()
        
        # 客户端连接 socket（初始为 None）
        self.client_8891 = None
        self.client_8892 = None
        self.client_8893 = None
        
        # 新增：用于记录 8893 端口是否已在当前 True 持续期间发送过指令
        self.sent_true_for_8893 = False   # <--- 新增

        # 创建话题订阅
        self.subscription = self.create_subscription(
            Bool,
            topic_name,
            self.callback,
            10
        )
        self.get_logger().info(f'Subscribed to {topic_name}')
        
        # 创建保存图像的目录
        self.save_dir = './saved_images'
        if not os.path.exists(self.save_dir):
            os.makedirs(self.save_dir)
            self.get_logger().info(f'Created directory {self.save_dir}')
        
        # 启动三个 TCP 服务器线程（8891、8892、8893）
        self.server_thread_8891 = threading.Thread(target=self.tcp_server_thread, args=(8891, 'client_8891'), daemon=True)
        self.server_thread_8892 = threading.Thread(target=self.tcp_server_thread, args=(8892, 'client_8892'), daemon=True)
        self.server_thread_8893 = threading.Thread(target=self.tcp_server_thread, args=(8893, 'client_8893'), daemon=True)
        self.server_thread_8891.start()
        self.server_thread_8892.start()
        self.server_thread_8893.start()
        
        self.get_logger().info('TCP servers started on ports 8891, 8892 and 8893, waiting for clients...')

    def tcp_server_thread(self, port, attr_name):
        """线程函数：在指定端口监听，接受一个客户端并保持连接"""
        server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_sock.bind(('0.0.0.0', port))
        server_sock.listen(1)
        self.get_logger().info(f'Listening on port {port}...')
        
        while rclpy.ok():
            try:
                client, addr = server_sock.accept()
                self.get_logger().info(f'New connection on port {port} from {addr}')
                with self.lock:
                    setattr(self, attr_name, client)
                
                # ---------- 端口8893专用：接收图像数据 ----------
                if port == 8893:
                    self.get_logger().info('Port 8893: Starting image receiving loop')
                    while rclpy.ok():
                        try:
                            # 1. 读取4字节长度（网络序）
                            len_data = b''
                            while len(len_data) < 4:
                                chunk = client.recv(4 - len(len_data))
                                if not chunk:
                                    raise Exception('Connection closed while reading length')
                                len_data += chunk
                            img_len = struct.unpack('>I', len_data)[0]  # 大端转本地
                            self.get_logger().debug(f'Expecting image of {img_len} bytes')
                            
                            # 2. 读取图像数据
                            img_data = b''
                            while len(img_data) < img_len:
                                chunk = client.recv(img_len - len(img_data))
                                if not chunk:
                                    raise Exception('Connection closed while reading image data')
                                img_data += chunk
                            
                            # 3. 解码图像
                            nparr = np.frombuffer(img_data, np.uint8)
                            img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                            if img is not None:
                                # 显示1秒
                                cv2.imshow('Received Image', img)
                                cv2.waitKey(1000)  # 毫秒
                                # 保存带时间戳
                                timestamp = time.strftime('%Y%m%d_%H%M%S', time.localtime())
                                filename = os.path.join(self.save_dir, f'image_{timestamp}.jpg')
                                cv2.imwrite(filename, img)
                                self.get_logger().info(f'Image saved as {filename}')
                            else:
                                self.get_logger().warn('Failed to decode image')
                        except Exception as e:
                            self.get_logger().error(f'Error receiving image on port 8893: {e}')
                            break
                    # 退出接收循环，关闭连接并置空
                    client.close()
                    with self.lock:
                        setattr(self, attr_name, None)
                    self.get_logger().warn('Client on port 8893 disconnected, waiting for new connection...')
                    continue  # 回到外循环重新accept
                
                # ---------- 端口8891和8892：仅检测断开 ----------
                else:
                    while rclpy.ok():
                        try:
                            client.settimeout(1.0)
                            data = client.recv(1024)
                            if not data:
                                break
                        except socket.timeout:
                            continue
                        except Exception:
                            break
                    # 连接断开清理
                    with self.lock:
                        setattr(self, attr_name, None)
                    self.get_logger().warn(f'Client on port {port} disconnected, waiting for new connection...')
                    client.close()
                    # 继续外循环，等待新连接
                    
            except Exception as e:
                self.get_logger().error(f'Error on port {port}: {e}')
                time.sleep(1)
        
        server_sock.close()

    def send_data(self, port, data_bytes):
        """通过指定端口对应的客户端连接发送数据（如果连接存在）"""
        if port == 8891:
            attr_name = 'client_8891'
        elif port == 8892:
            attr_name = 'client_8892'
        elif port == 8893:
            attr_name = 'client_8893'
        else:
            self.get_logger().error(f'Unsupported port {port}')
            return False
        
        with self.lock:
            client = getattr(self, attr_name)
        if client is None:
            self.get_logger().warn(f'No client connected on port {port}, data not sent')
            return False
        try:
            client.sendall(data_bytes)
            self.get_logger().debug(f'Sent {len(data_bytes)} bytes to port {port}')
            return True
        except Exception as e:
            self.get_logger().warn(f'Failed to send on port {port}: {e}')
            with self.lock:
                setattr(self, attr_name, None)
            try:
                client.close()
            except:
                pass
            return False

    def callback(self, msg):
        """话题回调：根据消息状态构造数据并发送"""
        if msg.data:   # True → 路径阻塞
            packet_8891 = struct.pack('BB', 2, 0)
            packet_8892 = b'1'
            # 发送到8891和8892（每次回调都发送）
            self.send_data(8891, packet_8891)
            self.send_data(8892, packet_8892)
            
            # 8893：仅在上升沿（从False变为True）时发送一次
            if not self.sent_true_for_8893:
                packet_8893 = b'a'
                self.send_data(8893, packet_8893)
                self.sent_true_for_8893 = True   # 标记已发送，防止连续True重复发送
        else:          # False → 路径通畅
            packet_8891 = struct.pack('BB', 0, 0)
            packet_8892 = b'2'
            self.send_data(8891, packet_8891)
            self.send_data(8892, packet_8892)
            # 重置8893触发标志，以便下次True时再次触发
            self.sent_true_for_8893 = False

def main(args=None):
    rclpy.init(args=args)
    node = PathBlockedTcpServer()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()