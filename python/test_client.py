import socket
import struct

# 消息类型
MSG_TYPE_LOGIN = 1
MSG_TYPE_LOGIN_RESP = 2
MSG_TYPE_REGISTER = 3
MSG_TYPE_REGISTER_RESP = 4
MSG_TYPE_CHAT = 5
MSG_TYPE_CHAT_RESP = 6
MSG_TYPE_ERROR = 9

def send_message(sock, msg_type, user_id, data):
    """发送消息到服务器"""
    try:
        # 计算消息长度
        header_size = 12  # 4+4+4
        data_size = len(data)
        total_length = header_size + data_size

        # 构建头部（网络字节序，大端序）
        header = struct.pack('!III', total_length, msg_type, user_id)

        # 发送消息
        sock.sendall(header + data.encode('utf-8'))
        print(f"发送消息: 类型={msg_type}, 用户ID={user_id}, 数据={data}")
        return True
    except Exception as e:
        print(f"发送消息失败: {e}")
        return False

def receive_message(sock, timeout=5):
    """从服务器接收消息"""
    # 设置超时
    sock.settimeout(timeout)

    try:
        # 接收头部
        header = sock.recv(12)
        if not header:
            print("连接已关闭（没有收到头部）")
            return None

        # 解析头部（网络字节序转主机字节序）
        total_length, msg_type, user_id = struct.unpack('!III', header)
        print(f"收到头部: 总长度={total_length}, 类型={msg_type}, 用户ID={user_id}")

        # 计算消息体长度
        body_length = total_length - 12
        if body_length > 0:
            # 接收消息体
            data = sock.recv(body_length)
            if not data:
                print("连接已关闭（没有收到消息体）")
                return None
            return msg_type, user_id, data.decode('utf-8')
        else:
            # 没有消息体
            return msg_type, user_id, ""
    except socket.timeout:
        print(f"接收超时（{timeout}秒）")
        return None
    except Exception as e:
        print(f"接收消息失败: {e}")
        return None

def test_register():
    """测试用户注册"""
    print("=== 测试用户注册 ===")

    # 创建连接
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)  # 设置连接超时
        sock.connect(('localhost', 8888))
        print("连接服务器成功")
    except Exception as e:
        print(f"连接服务器失败: {e}")
        return

    # 发送注册请求
    username = "testuser"
    password = "123456"
    if send_message(sock, MSG_TYPE_REGISTER, 0, f"{username}:{password}"):
        print("等待服务器响应...")
        # 接收响应
        msg = receive_message(sock)
        if msg:
            print(f"注册响应: 类型={msg[0]}, 用户ID={msg[1]}, 内容={msg[2]}")
        else:
            print("没有收到服务器响应")
    else:
        print("发送注册请求失败")

    # 关闭连接
    sock.close()
    print("连接已关闭")

def test_login():
    """测试用户登录"""
    print("\n=== 测试用户登录 ===")

    # 创建连接
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)  # 设置连接超时
        sock.connect(('localhost', 8888))
        print("连接服务器成功")
    except Exception as e:
        print(f"连接服务器失败: {e}")
        return

    # 发送登录请求
    username = "user1"  # 已在数据库中初始化的用户
    password = "123456"
    if send_message(sock, MSG_TYPE_LOGIN, 0, f"{username}:{password}"):
        print("等待服务器响应...")
        # 接收响应
        msg = receive_message(sock)
        if msg:
            print(f"登录响应: 类型={msg[0]}, 用户ID={msg[1]}, 内容={msg[2]}")
        else:
            print("没有收到服务器响应")
    else:
        print("发送登录请求失败")

    # 关闭连接
    sock.close()
    print("连接已关闭")

if __name__ == "__main__":
    test_register()
    test_login()
    print("\n测试完成")