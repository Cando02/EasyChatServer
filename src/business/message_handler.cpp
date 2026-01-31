//
// Created by Cando on 2026/1/30.
//
#include "../../include/business/message_handler.h"
#include "../../include/network/socket.h"
#include <mysql/mysql.h>
#include <iostream>

namespace easychat{
    MessageHandler::MessageHandler() :
    conn_pool_(ConnectionPool::getInstance()),
    user_manager_(UserManager::getInstance()){}

    MessageHandler::~MessageHandler() {}

    MessageHandler &MessageHandler::getInstance() {
        static MessageHandler instance;
        return instance;
    }

    void MessageHandler::init() {
        std::cout<<"MessageHandler initialized"<<std::endl;
    }

    bool MessageHandler::storeMessage(int sender_id, int receiver_id, const std::string &content, int message_type,
                                      int is_offline) {
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;

        std::string insert_sql = "insert into messages(sender_id,receiver_id,content,message_type,is_offline) values("
                +std::to_string(sender_id)+", "+std::to_string(receiver_id)+", '"+content+"', "
                +std::to_string(message_type)+", "+std::to_string(is_offline)+")";
        if (!conn->execute(insert_sql)){
            std::cerr<<"Failed to store message"<<std::endl;
            conn_pool_.returnConnection(conn);
            return false;
        }
        conn_pool_.returnConnection(conn);
        return true;
    }

    bool MessageHandler::forwardMessage(int receiver_id, const easychat::Message &msg) {
        // 获取接收者SocketFd
        int socket_fd = user_manager_.getSocketFdByUserId(receiver_id);
        if (socket_fd==-1) return false;
        // 序列化消息
        auto buffer = msg.serialize();
        // 创造Socket对象并发送信息，不拥有文件描述符
        Socket socket(socket_fd, false);
        ssize_t bytes_sent = socket.send(buffer.data(),buffer.size());

        if(bytes_sent==-1){
            std::cerr<<"Failed to forward message to user "<<receiver_id<<std::endl;
            return false;
        }
        std::cout<<"Message forwarded to user "<<receiver_id<<",bytes sent: "<<bytes_sent<<std::endl;
        return true;
    }
    bool MessageHandler::sendMessage(int sender_id, int receiver_id, const std::string &content, int message_type) {
        // 检查接收者是否在线
        bool is_online = user_manager_.isUserOnline(receiver_id);
        // 存储消息
        int is_offline = is_online ? 0 : 1 ;
        if (!storeMessage(sender_id,receiver_id,content,message_type,is_offline)) return false;
        // 接收者在线转发消息
        if (is_online){
            Message msg(MessageType::MSG_TYPE_CHAT,sender_id,content);
            forwardMessage(receiver_id,msg);
        }else{
            std::cout<<"Message stored as offline for user "<<receiver_id<<std::endl;
        }
        return true;
    }

    void MessageHandler::handleReceivedMessage(const easychat::Message &msg) {
        int sender_id = msg.getUserId();
        std::string content = msg.getData();

        // 解析消息内容
        size_t colon_pos = content.find(':');
        if (colon_pos==std::string::npos){
            std::cerr<<"Invalid message format: "<<content<<std::endl;
            return;
        }
        int receiver_id = std::stoi(content.substr(0,colon_pos));
        std::string message_content = content.substr(colon_pos+1);

        // 发送消息
        sendMessage(sender_id,receiver_id,message_content,static_cast<int>(msg.getType()));
    }
    bool MessageHandler::getOfflineMessage(int user_id, std::vector<MessageInfo> &messages) {
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;

        std::string query_sql = "select id,sender_id,receiver_id,content,message_type,is_offline,is_read,created_at "
                                "from messages where receiver_id="+std::to_string(user_id)+" and is_offline=1 order by created_at asc";
        MYSQL_RES* result = conn->query(query_sql);
        if (!result) {
            conn_pool_.returnConnection(conn);
            return false;
        }
        MYSQL_ROW row;
        while ((row= mysql_fetch_row(result))!= nullptr){
            MessageInfo msg_info;
            msg_info.id = std::stoi(row[0]);
            msg_info.sender_id = std::stoi(row[1]);
            msg_info.receiver_id = std::stoi(row[2]);
            msg_info.content = row[3];
            msg_info.message_type = std::stoi(row[4]);
            msg_info.is_offline = std::stoi(row[5]);
            msg_info.is_read = std::stoi(row[6]);
            messages.push_back(msg_info);
        }
        mysql_free_result(result);

        // 标记离线消息为已读
        std::string update_sql = "update messages set is_offline=0,is_read=1 where receiver_id= "
                +std::to_string(user_id)+" and is_offline=1";
        conn->execute(update_sql);

        conn_pool_.returnConnection(conn);
        return true;
    }

    bool MessageHandler::markMessageAsRead(int message_id) {
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;

        std::string update_sql = "update messages set is_read = 1 where id= "+std::to_string(message_id);
        bool result = conn->execute(update_sql);
        conn_pool_.returnConnection(conn);
        return result;
    }
    bool MessageHandler::getChatHistory(int user_id1, int user_id2, std::vector<MessageInfo> &messages, int limit) {
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;
        std::string query_sql = "select id,sender_id,receiver_id,content,message_type,is_offline,is_read,created_at"
                                "from messages where (sender_id="+std::to_string(user_id1)+" and receiver_id = "+std::to_string(user_id2)+") "
                                "or (sender_id="+std::to_string(user_id2)+" and receiver_id = "+std::to_string(user_id1)+") "
                                "order by created_at desc limit "+std::to_string(limit);
        MYSQL_RES *result = conn->query(query_sql);
        if (!result) {
            conn_pool_.returnConnection(conn);
            return false;
        }
        MYSQL_ROW row;
        while ((row= mysql_fetch_row(result))!= nullptr){
            MessageInfo msg_info;
            msg_info.id = std::stoi(row[0]);
            msg_info.sender_id = std::stoi(row[1]);
            msg_info.receiver_id = std::stoi(row[2]);
            msg_info.content = row[3];
            msg_info.message_type = std::stoi(row[4]);
            msg_info.is_offline = std::stoi(row[5]);
            msg_info.is_read = std::stoi(row[6]);
            msg_info.created_at = row[7];
            messages.push_back(msg_info);
        }
        mysql_free_result(result);
        conn_pool_.returnConnection(conn);
        return true;
    }
}