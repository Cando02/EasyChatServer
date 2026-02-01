"""
客户端使用示例
测试所有客户端功能
"""
import time
import sys
import os

# 添加父目录到Python路径
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from python.client import EasyChatClient

def main():
    # 创建客户端
    client = EasyChatClient('localhost', 8888)

    # 连接服务器
    if not client.connect():
        return

    print("=" * 60)
    print("测试1: 注册用户")
    print("=" * 60)
    # 注册用户
    client.register('testuser', '123456', '测试用户')
    time.sleep(1)

    print("=" * 60)
    print("测试2: 用户登录")
    print("=" * 60)
    # 登录
    client.login('testuser', '123456')
    time.sleep(1)

    print("=" * 60)
    print("测试3: 获取在线用户列表")
    print("=" * 60)
    # 获取在线用户列表（获取其他用户的用户名）
    client.get_online_users()
    time.sleep(1)

    print("=" * 60)
    print("测试4: 使用用户名发送消息")
    print("=" * 60)
    # 使用用户名发送消息
    for i in range(3):
        client.send_chat('user2', f"使用用户名发送的测试消息 {i+1}")
        time.sleep(0.5)

    print("=" * 60)
    print("测试5: 使用用户ID发送消息")
    print("=" * 60)
    # 使用用户ID发送消息（兼容旧方式）
    client.send_chat(2, "使用用户ID发送的测试消息")
    time.sleep(0.5)

    print("=" * 60)
    print("测试6: 使用用户名获取聊天记录")
    print("=" * 60)
    # 使用用户名获取聊天记录
    client.get_chat_history('user2', 10)
    time.sleep(1)

    print("=" * 60)
    print("测试7: 使用用户ID获取聊天记录")
    print("=" * 60)
    # 使用用户ID获取聊天记录（兼容旧方式）
    client.get_chat_history(2, 5)
    time.sleep(1)

    print("=" * 60)
    print("测试8: 再次获取在线用户列表")
    print("=" * 60)
    # 再次获取在线用户列表
    client.get_online_users()
    time.sleep(1)

    print("=" * 60)
    print("测试完成，准备关闭连接...")
    print("=" * 60)
    # 关闭连接
    time.sleep(2)
    client.close()

if __name__ == "__main__":
    print("=" * 60)
    print("EasyChat客户端功能测试")
    print("测试功能：注册、登录、在线用户列表、发送消息（用户名/ID）、聊天记录（用户名/ID）")
    print("=" * 60)
    main()
