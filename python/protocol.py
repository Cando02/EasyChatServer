"""
协议处理模块
"""

import struct

# 消息类型定义（与服务器端保持一致）
MSG_TYPE_LOGIN = 1          # 登录请求
MSG_TYPE_LOGIN_RESP = 2     # 登录响应
MSG_TYPE_REGISTER = 3       # 注册请求
MSG_TYPE_REGISTER_RESP = 4   # 注册响应
MSG_TYPE_CHAT = 5           # 聊天消息
MSG_TYPE_CHAT_RESP = 6      # 聊天响应
MSG_TYPE_OFFLINE_MSG = 7     # 离线消息
MSG_TYPE_HEARTBEAT = 8      # 心跳消息
MSG_TYPE_ERROR = 9          # 错误消息
MSG_TYPE_HISTORY = 10        # 聊天记录请求
MSG_TYPE_GET_USERS = 11      # 获取在线用户请求
MSG_TYPE_HISTORY_RESP = 12   # 聊天记录响应
MSG_TYPE_USERS_RESP = 13      # 在线用户响应
MSG_TYPE_GET_USER_BY_NAME = 14  # 根据用户名获取用户信息
MSG_TYPE_GET_USER_BY_NAME_RESP = 15  # 根据用户名获取用户信息响应

class MessageProtocol:
    """消息协议处理类"""
    HEADER_SIZE = 12

    @staticmethod
    def pack_message(msg_type,user_id,data):
        """打包消息为二进制数据"""
        data_bytes = data.encode('utf-8')
        data_size = len(data_bytes)
        total_length = MessageProtocol.HEADER_SIZE + data_size
        # 打包头部
        header = struct.pack('!III', total_length,msg_type,user_id)
        # 返回完整消息
        return header + data_bytes
    
    @staticmethod
    def unpack_header(header_bytes):
        """解包消息头"""
        if len(header_bytes) != MessageProtocol.HEADER_SIZE:
            raise ValueError(f"Invalid header size: {len(header_bytes)}")
        total_length,msg_type,user_id = struct.unpack('!III', header_bytes)
        return total_length,msg_type, user_id
    
    @staticmethod
    def get_message_type_name(msg_type):
        """获取消息类型名称"""
        type_name = {
            MSG_TYPE_LOGIN: 'LOGIN',
            MSG_TYPE_LOGIN_RESP: 'LOGIN_RESP',
            MSG_TYPE_REGISTER: 'REGISTER',
            MSG_TYPE_REGISTER_RESP: 'REGISTER_RESP',
            MSG_TYPE_CHAT: 'CHAT',
            MSG_TYPE_ERROR: 'ERROR',
            MSG_TYPE_HEARTBEAT: 'HEARTBEAT',
            MSG_TYPE_OFFLINE_MSG: 'OFFLINE_MSG',
            MSG_TYPE_HISTORY: 'HISTORY',
            MSG_TYPE_HISTORY_RESP: 'HISTORY_RESP',
            MSG_TYPE_GET_USERS: 'GET_USERS',
            MSG_TYPE_USERS_RESP: 'GET_USERS_RESP',
            MSG_TYPE_GET_USER_BY_NAME: 'GET_USER_BY_NAME',
            MSG_TYPE_GET_USER_BY_NAME_RESP: 'GET_USER_BY_NAME_RESP'
        }
        return type_name.get(msg_type,f"UNKNOWN({msg_type})")
