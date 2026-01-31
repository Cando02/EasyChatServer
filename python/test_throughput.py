import socket
import struct
import threading
import time

# 消息类型
MSG_TYPE_LOGIN = 1
MSG_TYPE_CHAT = 5

def send_message(sock, msg_type, user_id, data):
    """发送消息到服务器"""
    header_size = 12
    data_size = len(data)
    total_length = header_size + data_size
    header = struct.pack('!III', total_length, msg_type, user_id)
    sock.sendall(header + data.encode('utf-8'))

def receive_message(sock):
    """从服务器接收消息"""
    header = sock.recv(12)
    if not header:
        return None
    total_length, msg_type, user_id = struct.unpack('!III', header)
    data = sock.recv(total_length - 12)
    if not data:
        return None
    return msg_type, user_id, data.decode('utf-8')

def sender_thread(sock, user_id, receiver_id, count):
    """发送消息的线程"""
    start_time = time.time()
    for i in range(count):
        send_message(sock, MSG_TYPE_CHAT, user_id, f"{receiver_id}:Message {i}")
    end_time = time.time()
    print(f"发送完成，耗时: {end_time - start_time:.2f} 秒")
    print(f"发送速率: {count / (end_time - start_time):.2f} 消息/秒")

def receiver_thread(sock, count):
    """接收消息的线程"""
    received = 0
    start_time = time.time()
    while received < count:
        msg = receive_message(sock)
        if msg:
            received += 1
    end_time = time.time()
    print(f"接收完成，耗时: {end_time - start_time:.2f} 秒")
    print(f"接收速率: {received / (end_time - start_time):.2f} 消息/秒")

def test_throughput():
    """测试消息吞吐量"""
    print("=== 测试消息吞吐量 ===")

    # 用户1连接并登录
    sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock1.connect(('localhost', 8888))
    send_message(sock1, MSG_TYPE_LOGIN, 0, "user1:123456")
    msg1 = receive_message(sock1)
    user1_id = msg1[1]
    print(f"user1 登录成功，ID={user1_id}")

    # 用户2连接并登录
    sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock2.connect(('localhost', 8888))
    send_message(sock2, MSG_TYPE_LOGIN, 0, "user2:123456")
    msg2 = receive_message(sock2)
    user2_id = msg2[1]
    print(f"user2 登录成功，ID={user2_id}")

    # 测试参数
    message_count = 1000  # 发送1000条消息

    # 启动接收线程
    receiver = threading.Thread(target=receiver_thread, args=(sock2, message_count))
    receiver.start()

    # 启动发送线程
    sender = threading.Thread(target=sender_thread, args=(sock1, user1_id, user2_id, message_count))
    sender.start()

    # 等待线程完成
    sender.join()
    receiver.join()

    # 关闭连接
    sock1.close()
    sock2.close()
    print("测试完成")

if __name__ == "__main__":
    test_throughput()