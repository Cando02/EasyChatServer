import socket
import struct
import time

# 消息类型
MSG_TYPE_LOGIN = 1
MSG_TYPE_LOGIN_RESP = 2
MSG_TYPE_REGISTER = 3
MSG_TYPE_REGISTER_RESP = 4
MSG_TYPE_CHAT = 5
MSG_TYPE_CHAT_RESP = 6
MSG_TYPE_OFFLINE_MSG = 7
MSG_TYPE_HEARTBEAT = 8
MSG_TYPE_ERROR = 9

def send_message(sock, msg_type, user_id, data):
    """发送消息到服务器"""
    header_size = 12
    data_size = len(data)
    total_length = header_size + data_size
    header = struct.pack('!III', total_length, msg_type, user_id)
    sock.sendall(header + data.encode('utf-8'))
    print(f"发送消息: 类型={msg_type}, 用户ID={user_id}, 内容={data}")

def receive_message(sock, timeout=5):
    """从服务器接收消息"""
    sock.settimeout(timeout)
    try:
        header = sock.recv(12)
        if not header:
            print("连接已关闭")
            return None
        total_length, msg_type, user_id = struct.unpack('!III', header)
        data = sock.recv(total_length - 12)
        if not data:
            print("未收到数据")
            return None
        content = data.decode('utf-8')
        print(f"收到消息: 类型={msg_type}, 用户ID={user_id}, 内容={content}")
        return msg_type, user_id, content
    except socket.timeout:
        print("接收消息超时")
        return None
    except Exception as e:
        print(f"接收消息错误: {e}")
        return None

def test_offline_message():
    """测试离线消息"""
    print("=== 测试离线消息 ===")

    # 用户2先登录获取ID，然后断开连接
    print("user2 先登录获取ID...")
    sock_temp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock_temp.connect(('localhost', 8888))
    send_message(sock_temp, MSG_TYPE_LOGIN, 0, "user2:123456")
    msg_temp = receive_message(sock_temp)
    if not msg_temp:
        print("user2 登录失败")
        return
    user2_id = msg_temp[1]
    print(f"user2 登录成功，ID={user2_id}")
    sock_temp.close()
    print("user2 已断开连接\n")

    # 等待一下，确保服务器已经处理完断开连接
    time.sleep(1)

    # 用户1连接并登录
    sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock1.connect(('localhost', 8888))
    send_message(sock1, MSG_TYPE_LOGIN, 0, "user1:123456")
    msg1 = receive_message(sock1)
    if not msg1:
        print("user1 登录失败")
        return
    user1_id = msg1[1]
    print(f"user1 登录成功，ID={user1_id}")

    # 用户1发送消息给离线的用户2
    print(f"\nuser1 发送消息给离线的 user2(ID={user2_id})...")
    send_message(sock1, MSG_TYPE_CHAT, user1_id, f"{user2_id}:Hello, user2! I sent this when you were offline.")
    print("消息发送成功，服务器会存储为离线消息")

    # 等待一下，确保服务器已经处理完消息
    time.sleep(1)

    # 用户1断开连接
    sock1.close()
    print("user1 已断开连接")

    # 等待一下，确保服务器已经处理完断开连接
    time.sleep(1)

    # 用户2连接并登录，应该收到离线消息
    print("\nuser2 登录...")
    sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock2.connect(('localhost', 8888))
    send_message(sock2, MSG_TYPE_LOGIN, 0, "user2:123456")
    msg2 = receive_message(sock2)
    if not msg2:
        print("user2 登录失败")
        return
    print(f"user2 登录成功，ID={msg2[1]}")

    # 接收离线消息
    print("\n等待接收离线消息...")
    start_time = time.time()
    timeout = 10  # 10秒超时
    while time.time() - start_time < timeout:
        msg = receive_message(sock2, timeout=2)
        if not msg:
            break
        if msg[0] == MSG_TYPE_OFFLINE_MSG:
            print(f"收到离线消息: 发送者ID={msg[1]}, 内容={msg[2]}")
            # 收到离线消息后退出循环
            break
        else:
            print(f"收到其他消息: 类型={msg[0]}, 用户ID={msg[1]}, 内容={msg[2]}")

    # 关闭连接
    sock2.close()
    print("测试完成")

if __name__ == "__main__":
    test_offline_message()
