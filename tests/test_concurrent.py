import socket
import struct
import threading
import time

# 消息类型
MSG_TYPE_LOGIN = 1
MSG_TYPE_HEARTBEAT = 8

def send_message(sock, msg_type, user_id, data):
    """发送消息到服务器"""
    header_size = 12
    data_size = len(data)
    total_length = header_size + data_size
    header = struct.pack('!III', total_length, msg_type, user_id)
    sock.sendall(header + data.encode('utf-8'))

def receive_message(sock):
    """从服务器接收消息"""
    try:
        header = sock.recv(12, socket.MSG_DONTWAIT)
        if not header:
            return None
        total_length, msg_type, user_id = struct.unpack('!III', header)
        data = sock.recv(total_length - 12, socket.MSG_DONTWAIT)
        if not data:
            return None
        return msg_type, user_id, data.decode('utf-8')
    except BlockingIOError:
        return None

def client_thread(client_id):
    """客户端线程"""
    try:
        # 创建连接
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('localhost', 8888))
        sock.setblocking(False)  # 设置为非阻塞模式

        # 登录
        username = f"user{client_id}"
        password = "123456"
        send_message(sock, MSG_TYPE_LOGIN, 0, f"{username}:{password}")

        # 等待登录响应
        start_time = time.time()
        while time.time() - start_time < 5:
            msg = receive_message(sock)
            if msg:
                break
            time.sleep(0.1)

        # 发送心跳消息
        while time.time() - start_time < 30:  # 运行30秒
            send_message(sock, MSG_TYPE_HEARTBEAT, client_id, "ping")
            time.sleep(1)

        # 关闭连接
        sock.close()
        print(f"客户端 {client_id} 测试完成")
    except Exception as e:
        print(f"客户端 {client_id} 测试失败: {e}")

def test_concurrent_connections(count):
    """测试并发连接"""
    print(f"=== 测试并发连接数: {count} ===")

    threads = []
    start_time = time.time()

    # 创建客户端线程
    for i in range(1, count + 1):
        t = threading.Thread(target=client_thread, args=(i,))
        threads.append(t)
        t.start()
        # 间隔一段时间创建连接，避免同时创建过多连接
        if i % 100 == 0:
            time.sleep(0.5)

    # 等待所有线程完成
    for t in threads:
        t.join()

    end_time = time.time()
    print(f"测试完成，耗时: {end_time - start_time:.2f} 秒")
    print(f"并发连接数: {count}")

if __name__ == "__main__":
    # 测试不同并发连接数
    test_concurrent_connections(100)  # 100个并发连接
    test_concurrent_connections(500)  # 500个并发连接
    test_concurrent_connections(1000)  # 1000个并发连接