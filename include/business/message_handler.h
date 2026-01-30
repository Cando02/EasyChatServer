//
// Created by Cando on 2026/1/30.
//

#ifndef EASYCHATSERVER_MESSAGE_HANDLER_H
#define EASYCHATSERVER_MESSAGE_HANDLER_H

#include "common/protocol.h"
#include "database/connection_pool.h"
#include "business/user_manager.h"
#include <string>
#include <vector>

namespace easychat{
    // 消息信息结构体
    struct MessageInfo{
        int id;
        int sender_id;
        int receiver_id;
        std::string content;
        int message_type; // 0-文本，1-图片，2-文件
        int is_offline; // 0-否，1-是
        int is_read; //0-未读，1-已读
        std::string created_at;
    };
    // 消息处理类
    class MessageHandler{
    public:
        static MessageHandler& getInstance();
        // 初始化
        void init();
        // 发送消息
        bool sendMessage(int sender_id,int receiver_id,
                         const std::string &content,int message_type=0);
        // 处理接收消息
        void handleReceivedMessage(const Message& msg);
        // 获取离线消息
        bool getOfflineMessage(int user_id,std::vector<MessageInfo>&messages);
        // 标记消息已读
        bool markMessageAsRead(int message_id);
        // 获取用户聊天记录
        bool getChatHistory(int user_id1,int user_id2,std::vector<MessageInfo>&messages,int limit=100);
    private:
        MessageHandler();
        ~MessageHandler();
        // 禁止拷贝和赋值
        MessageHandler(const MessageHandler&) = delete;
        MessageHandler& operator=(const MessageHandler&) = delete;
        // 存储消息到数据库
        bool storeMessage(int sender_id,int receiver_id,const std::string& content,
                          int message_type,int is_offline);
        // 转发消息到在线用户
        bool forwardMessage(int receiver_id,const Message& msg);
        // 连接池引用
        ConnectionPool& conn_pool_;
        // 用户管理引用
        UserManager& user_manager_;
    };
}

#endif //EASYCHATSERVER_MESSAGE_HANDLER_H
