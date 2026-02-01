"""
Python客户端主程序
"""
import socket
import threading
import time
import sys

from protocol import MessageProtocol,MSG_TYPE_LOGIN,MSG_TYPE_CHAT,MSG_TYPE_ERROR,MSG_TYPE_OFFLINE_MSG,MSG_TYPE_LOGIN_RESP,MSG_TYPE_REGISTER,MSG_TYPE_HISTORY,MSG_TYPE_GET_USERS,MSG_TYPE_HISTORY_RESP,MSG_TYPE_GET_USER_BY_NAME,MSG_TYPE_GET_USER_BY_NAME_RESP,MSG_TYPE_USERS_RESP

class EasyChatClient:
    """EasyChat客户端类"""
    def __init__(self,host='localhost',port=8888):
        """初始化客户端"""
        self.host = host
        self.port = port
        self.sock = None        # sock对象
        self.connected = False  # 连接状态
        self.user_id = -1       # 用户ID(未登录为-1)
        self.username = ""      # 当前登录的用户名
        self.receive_thread = None #接收线程
        self.running = False    #运行标志
        self.message_callback = None #消息回调函数
        self.user_map = {}      # 用户名到ID的映射
        self.online_users = {}   # 在线用户列表 {user_id: username}

    def connect(self):
        """连接服务器"""
        try:
            #创建TCP
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            #设置超时时间
            self.sock.settimeout(10)
            #连接服务器
            self.sock.connect((self.host,self.port))
            #连接成功，恢复为非阻塞模式
            self.sock.settimeout(None)

            self.connected = True
            self.running = True

            print(f"✔ 连接成功：{self.host}:{self.port}")

            #启动接收线程
            self.receive_thread = threading.Thread(target=self._receive_loop)
            self.receive_thread.daemon = True
            self.receive_thread.start()

            return True
        except socket.timeout:
            print(f"❌ 连接超时：{self.host}:{self.port}")
            return False
        except ConnectionResetError:
            print(f"❌ 连接被拒绝：{self.host}:{self.port}")
            return False
        except Exception as e:
            print(f"❌ 连接失败：{e}")
            return False

    def login(self,username,password):
        """用户登录"""
        if not self.connected:
            print("❌ 未连接服务器")
            return False
        #构造登陆数据
        data = f"{username}:{password}"
        # 打包并发送消息
        message = MessageProtocol.pack_message(MSG_TYPE_LOGIN,0,data)
        self._send_raw(message)
        # 保存用户名
        self.username = username
        print(f"-> 发送登录请求：{username}")
        return True

    def register(self,username,password,nickname=""):
        """用户注册"""
        if not self.connected:
            print("❌ 未连接服务器")
            return False
        #构造注册数据
        data = f"{username}:{password}:{nickname}"
        #打包并发送消息
        message = MessageProtocol.pack_message(MSG_TYPE_REGISTER,0,data)
        self._send_raw(message)

        print(f"-> 发送注册请求：{username}")
        return True

    def send_chat(self,receiver,content):
        """发送聊天消息（支持用户名或用户ID）"""
        if not self.connected:
            print("❌ 未连接服务器")
            return False
        if self.user_id == -1:
            print("❌ 请登录")
            return False

        # 判断receiver是用户名还是用户ID
        try:
            receiver_id = int(receiver)
        except ValueError:
            # 是用户名，查找对应的ID
            receiver_id = self._get_user_id_by_name(receiver)
            if receiver_id == -1:
                print(f"❌ 未找到用户：{receiver}")
                return False

        #构造聊天数据
        data = f"{receiver_id}:{content}"
        #打包并发送消息
        message = MessageProtocol.pack_message(MSG_TYPE_CHAT,self.user_id,data)
        self._send_raw(message)

        receiver_name = self._get_user_name_by_id(receiver_id)
        print(f"-> 发送消息到{receiver_name}({receiver_id})：{content}")
        return True

    def get_chat_history(self,receiver,limit=50):
        """
        获取聊天记录（支持用户名或用户ID）
        """
        if not self.connected or self.user_id == -1:
            print("✗ 未登录或未连接")
            return False

        # 判断receiver是用户名还是用户ID
        try:
            receiver_id = int(receiver)
        except ValueError:
            # 是用户名，查找对应的ID
            receiver_id = self._get_user_id_by_name(receiver)
            if receiver_id == -1:
                print(f"❌ 未找到用户：{receiver}")
                return False

        # 构造请求数据
        data = f"{receiver_id}:{limit}"

        # 打包并发送消息
        message = MessageProtocol.pack_message(MSG_TYPE_HISTORY, self.user_id, data)
        self._send_raw(message)

        receiver_name = self._get_user_name_by_id(receiver_id)
        print(f"-> 请求与{receiver_name}({receiver_id})的聊天记录，最近{limit}条")
        return True

    def get_online_users(self):
        """
        获取在线用户列表
        """
        if not self.connected or self.user_id == -1:
            print("✗ 未登录或未连接")
            return False

        # 打包并发送消息
        message = MessageProtocol.pack_message(MSG_TYPE_GET_USERS, self.user_id, "")
        self._send_raw(message)

        print(f"-> 请求在线用户列表")

        return True

    def _get_user_id_by_name(self,username):
        """根据用户名获取用户ID"""
        # 先从本地缓存查找
        user_id = self.user_map.get(username, -1)
        if user_id != -1:
            return user_id
        
        # 本地缓存中找不到，向服务器查询
        print(f"-> 向服务器查询用户：{username}")
        message = MessageProtocol.pack_message(MSG_TYPE_GET_USER_BY_NAME, self.user_id, username)
        self._send_raw(message)
        
        # 等待服务器响应（这里使用简单的轮询，实际项目中应该使用更优雅的方式）
        import time
        start_time = time.time()
        timeout = 3  # 3秒超时
        while time.time() - start_time < timeout:
            user_id = self.user_map.get(username, -1)
            if user_id != -1:
                return user_id
            time.sleep(0.1)
        
        # 超时，返回-1
        return -1

    def _get_user_name_by_id(self,user_id):
        """根据用户ID获取用户名"""
        if user_id in self.online_users:
            return self.online_users[user_id]
        # 如果不在在线列表中，返回ID
        return str(user_id)

    def _send_raw(self,data):
        """发送原始二进制数据"""
        try:
            self.sock.sendall(data)
        except Exception as e:
            print(f"❌ 发送消息失败：{e}")
            self.connected = False

    def _receive_loop(self):
        """接收消息循环"""
        while self.running and self.connected:
            try:
                # 接收消息头部
                header = self._recv_exact(MessageProtocol.HEADER_SIZE)
                if not header:
                    print("❌ 连接被关闭（头部）")
                    self.connected = False
                    break
                #解包头部
                total_length,msg_type,user_id = MessageProtocol.unpack_header(header)
                #接收消息数据
                data_size = total_length-MessageProtocol.HEADER_SIZE
                if data_size>0:
                    data = self._recv_exact(data_size)
                    if not data:
                        print("❌ 连接被关闭（数据）")
                        self.connected = False
                        break
                    data_str = data.decode('utf-8')
                else:
                    data_str = ""
                #处理消息
                self._handle_message(msg_type,user_id,data_str)
            except Exception as e:
                print(f"❌ 接收消息失败：{e}")
                self.connected = False
                break

    def _recv_exact(self,size):
        """接收指定字节数的数据"""
        data = b''
        while len(data) < size:
            chunk = self.sock.recv(size-len(data))
            if not chunk:
                return None
            data += chunk
        return data

    def _handle_message(self,msg_type,user_id,data):
        """处理接收到的消息"""
        msg_type_name = MessageProtocol.get_message_type_name(msg_type)
        print(f"<- 接收消息[{msg_type_name}] from {user_id}:{data}")

        if msg_type==MSG_TYPE_LOGIN_RESP:
            #登陆响应
            if user_id != -1:
                self.user_id = user_id
                print(f"✔ 登陆成功，用户ID:{user_id}")
            else:
                print(f"❌ 登陆失败：{data}")
        elif msg_type==MSG_TYPE_CHAT:
            # 聊天消息
            if self.message_callback:
                self.message_callback('chat',user_id,data)
        elif msg_type==MSG_TYPE_OFFLINE_MSG:
            #离线消息
            print(f"📫 发离线消息 from {user_id}:{data}")
            if self.message_callback:
                self.message_callback('offline',user_id,data)
        elif msg_type==MSG_TYPE_USERS_RESP or msg_type==13: # 13是MSG_TYPE_USERS_RESP
            # 在线用户列表响应
            self._handle_online_users_response(data)
        elif msg_type==MSG_TYPE_HISTORY_RESP:
            # 聊天记录响应
            self._handle_history_response(data)
        elif msg_type==MSG_TYPE_GET_USER_BY_NAME_RESP:
            # 根据用户名获取用户信息响应
            self._handle_get_user_by_name_response(data)
        elif msg_type==MSG_TYPE_ERROR:
            #错误消息
            print(f"⚠ 错误：{data}")

    def _handle_online_users_response(self,data):
        """处理在线用户列表响应"""
        try:
            # 解析用户列表（格式：user_id:username,user_id:username,...）
            self.online_users.clear()
            if data:
                user_entries = data.split(',')
                for entry in user_entries:
                    if ':' in entry:
                        user_id_str, username = entry.split(':', 1)
                        user_id = int(user_id_str)
                        self.online_users[user_id] = username
                        self.user_map[username] = user_id

            # 显示在线用户列表
            print(f"✔ 在线用户列表（共{len(self.online_users)}人）：")
            for user_id, username in self.online_users.items():
                print(f"  - {username} (ID: {user_id})")
        except Exception as e:
            print(f"❌ 解析在线用户列表失败：{e}")

    def _handle_history_response(self,data):
        """处理聊天记录响应"""
        try:
            # 解析聊天记录（格式：sender_id:content|sender_id:content|...）
            if data:
                messages = data.split('|')
                print(f"✔ 聊天记录（共{len(messages)}条）：")
                for i, msg in enumerate(messages, 1):
                    if ':' in msg:
                        sender_id_str, content = msg.split(':', 1)
                        sender_id = int(sender_id_str)
                        sender_name = self._get_user_name_by_id(sender_id)
                        print(f"  {i}. [{sender_name}] {content}")
            else:
                print("✔ 聊天记录：无")
        except Exception as e:
            print(f"❌ 解析聊天记录失败：{e}")

    def _handle_get_user_by_name_response(self,data):
        """处理根据用户名获取用户信息响应"""
        try:
            # 解析用户信息（格式：user_id:username）
            if data and ':' in data:
                user_id_str, username = data.split(':', 1)
                user_id = int(user_id_str)
                # 缓存到本地
                self.user_map[username] = user_id
                self.online_users[user_id] = username
                print(f"✔ 找到用户：{username} (ID: {user_id})")
            else:
                print("❌ 未找到用户")
        except Exception as e:
            print(f"❌ 解析用户信息失败：{e}")

    def set_message_callback(self,callback):
        """设置消息回调函数"""
        self.message_callback = callback

    def close(self):
        """关闭连接"""
        self.running = False
        self.connected = False
        if self.sock:
            try:
                self.sock.close()
            except:
                pass
        if self.receive_thread:
            self.receive_thread.join(timeout=2)
        print("✔ 连接已关闭")

def interactive_client():
    """交互式客户端"""
    client = EasyChatClient('localhost',8888)
    if not client.connect():
        return

    #设置消息回调
    def on_message(msg_type,user_id,data):
        print(f"\n[新消息] {msg_type}:{data}\n>",end='',flush=True)
    client.set_message_callback(on_message)

    #主循环
    while True:
        try:
            #显示提示符
            print("> ",end='',flush=True)
            cmd = input().strip()
            if not cmd:
                continue
            #解析命令
            if cmd.startswith('login '):
                #登陆命令
                parts = cmd.split(' ',2)
                if len(parts) == 3:
                    client.login(parts[1],parts[2])
                    time.sleep(1)#等待响应
            elif cmd.startswith('register '):
                #注册命令
                parts = cmd.split(' ',3)
                if len(parts) >= 3:
                    nickname = parts[3] if len(parts)==4 else ""
                    client.register(parts[1],parts[2],nickname)
                    time.sleep(1)
            elif cmd.startswith('send '):
                # 发送命令（支持用户名或用户ID）
                parts = cmd.split(' ',2)
                if len(parts) == 3:
                    client.send_chat(parts[1],parts[2])
            elif cmd.startswith('history '):
                # 查看聊天记录（支持用户名或用户ID）
                parts = cmd.split(' ',2)
                if len(parts) >= 2:
                    limit = int(parts[2]) if len(parts) == 3 else 50
                    client.get_chat_history(parts[1], limit)
            elif cmd == 'users':
                # 查看在线用户
                client.get_online_users()
            elif cmd=='quit' or cmd=='exit':
                #退出命令
                break
            else:
                print("可用命令:")
                print("  login username password    - 登录")
                print("  register username password [nickname] - 注册")
                print("  send receiver message    - 发送消息（支持用户名或用户ID）")
                print("  history user [limit]     - 查看聊天记录（支持用户名或用户ID）")
                print("  users                     - 查看在线用户")
                print("  quit                      - 退出")
        except KeyboardInterrupt:
            print("\n")
            break
        except Exception as e:
            print(f"❌ 命令执行失败：{e}")
    client.close()

if __name__ == "__main__":
    print("=" * 50)
    print("EasyChat Python客户端")
    print("=" * 50)
    print("可用命令:")
    print("  login username password    - 登录")
    print("  register username password [nickname] - 注册")
    print("  send receiver message    - 发送消息（支持用户名或用户ID）")
    print("  history user [limit]     - 查看聊天记录（支持用户名或用户ID）")
    print("  users                     - 查看在线用户")
    print("  quit                      - 退出")
    print("=" * 50)

    interactive_client()
