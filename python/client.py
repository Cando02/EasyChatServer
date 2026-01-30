import socket
import struct

# 消息类型
MSG_TYPE_LOGIN = 1
MSG_TYPE_LOGIN_RESP = 2
MSG_TYPE_CHAT = 5

def send_message(sock, msg_type, user_id, data):
    # 计算消息长度
    header_size = 12  # 4+4+4
    data_size = len(data)
    total_length = header_size + data_size

    # 构建头部
    header = struct.pack('!III', total_length, msg_type, user_id)

    # 发送消息
    sock.sendall(header + data.encode('utf-8'))

def receive_message(sock):
    # 接收头部
    header = sock.recv(12)
    if not header:
        return None

    # 解析头部
    total_length, msg_type, user_id = struct.unpack('!III', header)

    # 接收消息体
    data = sock.recv(total_length - 12)
    if not data:
        return None

    return msg_type, user_id, data.decode('utf-8')

def main():
    # 连接服务器
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 8888))
    print("Connected to server")

    # 登录
    print("Logging in...")
    send_message(sock, MSG_TYPE_LOGIN, 0, 'user1:123456')
    msg = receive_message(sock)
    if msg:
        print(f"Login response: {msg[2]}")

    # 发送消息
    print("Sending message...")
    send_message(sock, MSG_TYPE_CHAT, 2, '1:Hello from user1!')
    print("Message sent")

    # 接收消息
    while True:
        msg = receive_message(sock)
        if msg:
            print(f"Received message: type={msg[0]}, user={msg[1]}, content={msg[2]}")

    # 关闭连接
    sock.close()

if __name__ == "__main__":
    main()