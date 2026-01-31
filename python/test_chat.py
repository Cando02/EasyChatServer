import socket
import struct
import threading

#消息类型
MSG_TYPE_LOGIN = 1
MSG_TYPE_CHAT = 5

def send_message(sock, msg_type, user_id, data):
    """发送消息到服务器"""
    # 计算消息长度
    header_size = 12  # 4+4+4
    data_size = len(data)
    total_length = header_size + data_size

    # 构建头部（网络字节序，大端序）
    header = struct.pack('!III', total_length, msg_type, user_id)

    # 发送消息
    sock.sendall(header + data.encode('utf-8'))

def receive_message(sock, timeout=5):
    """从服务器接收消息"""
    # 接收头部
    header = sock.recv(12)
    if not header:
        print("连接已关闭（没有收到头部）")
        return None

    # 解析头部（网络字节序转主机字节序）
    total_length, msg_type, user_id = struct.unpack('!III', header)
    # 接收消息体
    data = sock.recv(total_length-12)
    if not data:
        print("连接已关闭（没有收到消息体）")
        return None
    return msg_type, user_id, data.decode('utf-8')

def receive_thread(sock, username):
    """接收消息的线程"""
    while True:
        msg = receive_message(sock)
        if msg:
            print(f"[{username} 收到] 类型={msg[0]},用户ID={msg[1]},内容={msg[2]}")

def test_chat():
    """测试消息转发"""
    print("测试消息转发")
    # 用户1连接
    sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock1.connect(('localhost', 8888))
    #用户2连接
    sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock2.connect(('localhost', 8888))

    #用户1登陆
    send_message(sock1,MSG_TYPE_LOGIN, 0, "user1:123456")
    msg1 = receive_message(sock1)
    user1_id = msg1[1]
    print(f"user1 登陆成功，ID={user1_id}")
    #用户2登陆
    send_message(sock2,MSG_TYPE_LOGIN, 0, "user2:123456")
    msg2 = receive_message(sock2)
    user2_id = msg2[1]
    print(f"user2 登陆成功，ID={user2_id}")

    #启动接收线程
    thread1 = threading.Thread(target=receive_thread, args=(sock1,"user1"))
    thread1.daemon = True
    thread1.start()
    thread2 = threading.Thread(target=receive_thread, args=(sock2,"user2"))
    thread2.daemon = True
    thread2.start()

    #用户1发送消息给用户2
    print("\nuser1 发送消息给 user2...")
    send_message(sock1,MSG_TYPE_CHAT,user1_id, f"{user2_id}:Hello from user1!")

    #用户2发送消息给用户1
    print("\nuser2 发送消息给 user1...")
    send_message(sock2,MSG_TYPE_CHAT,user2_id, f"{user1_id}:Hello from user2!")

    #等待消息接收
    input("\n按回车结束测试...\n")
    #关闭连接
    sock1.close()
    sock2.close()

if __name__ == "__main__":
    test_chat()