//
// Created by 30501 on 2026/1/29.
//

#ifndef EASYCHATSERVER_PROTOCOL_H
#define EASYCHATSERVER_PROTOCOL_H

#include<cstdint>
#include<string>
#include<vector>
#include<arpa/inet.h>

namespace easychat {
// 消息类型
    enum class MessageType : uint32_t {
        MSG_TYPE_LOGIN = 1, // 登陆请求
        MSG_TYPE_LOGIN_RESP, // 登陆响应
        MSG_TYPE_REGISTER,  // 注册请求
        MSG_TYPE_REGISTER_RESP, // 注册响应
        MSG_TYPE_CHAT, // 聊天信息
        MSG_TYPE_CHAT_RESP, // 聊天响应
        MSG_TYPE_OFFLINE_MSG, // 离线消息推送
        MSG_TYPE_HEARTBEAT, // 心跳消息
        MSG_TYPE_ERROR, // 错误消息
        MSG_TYPE_HISTORY,        // 获取聊天记录
        MSG_TYPE_GET_USERS,      // 获取在线用户
        MSG_TYPE_HISTORY_RESP,    // 聊天记录响应
        MSG_TYPE_USERS_RESP,      // 在线用户响应
        MSG_TYPE_GET_USER_BY_NAME,  // 根据用户名获取用户信息
        MSG_TYPE_GET_USER_BY_NAME_RESP  // 根据用户名获取用户信息响应
    };
// 消息头部结构（固定12字节）
#pragma pack(push ,1)
    struct MessageHeader {
        uint32_t length; // 消息总长度（包含头部），4字节
        uint32_t type;  // 消息类型，4字节
        uint32_t user_id;   // 用户ID，4字节
    };
#pragma pack(pop)

// 消息类
    class Message {
    public:
        Message();

        explicit Message(MessageType type);

        Message(MessageType type,
                uint32_t user_id, const std::string &data);

        // 序列化：将消息转换为字节流
        std::vector<char> serialize() const;

        // 反序列化：从字节流恢复消息对象
        static Message deserialize(const char *data, size_t length);

        // Getter 方法 获取数据
        MessageType getType() const {
            return type_;
        }

        uint32_t getUserId() const {
            return user_id_;
        }

        const std::string &getData() const {
            return data_;
        }

        size_t getTotalLength() const;

        // Setter 方法 信息同步
        void setType(MessageType type) { type_ = type; }

        void setUserId(uint32_t user_id) { user_id_ = user_id; }

        void setData(const std::string &data) { data_ = data; }

    private:
        MessageType type_;
        uint32_t user_id_;
        std::string data_;
    };
// 工具函数：将主机字节序转换为网络字节序
// htonl: host to network long (32位整数)
    inline uint32_t hostToNetwork32(uint32_t host){
        return htonl(host);
    }
// 工具函数：将网络字节序转换为主机字节序
// ntohl: network to host long
    inline uint32_t networkTOHost32(uint32_t network){
        return ntohl(network);
    }
}
#endif //EASYCHATSERVER_PROTOCOL_H
