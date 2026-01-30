//
// Created by Cando on 2026/1/30.
//

#ifndef EASYCHATSERVER_REACTOR_H
#define EASYCHATSERVER_REACTOR_H

#include "network/epoll.h"
#include "network/socket.h"
#include "threadpool/threadpool.h"
#include "business/user_manager.h"
#include "business/message_handler.h"
#include <unordered_map>
#include <memory>

namespace easychat{
    // 客户端连接类
    class ClientConnection{
    public:
        ClientConnection(int fd,const std::string& ip ,int port);
        ~ClientConnection();

        // 处理可读事件
        void handleRead();
        // 处理可写事件
        void handleWrite();
        // 处理错误事件
        void handleError();
        // 处理关闭事件
        void handleClose();
        // 发送消息
        void sendMessage(const Message& msg);
        // 获取SocketFd
        int getFd() const {return fd_;}
        // 获取IP地址
        const std::string& getIp() const {return ip_;}
        // 获取端口
        int getPort() const {return port_;}
        // 获取用户ID
        int getUserId() const {return user_id_;}
        // 设置用户ID
        void setUserId(int user_id){user_id_=user_id;}
        // 检查是否认证
        bool isAuthenticated() const{return user_id_!=-1;}
    private:
        int fd_; //socket文件描述符
        std::string ip_;    //客户端IP地址
        int port_;  // 客户端端口
        int user_id_;   //用户ID(未认证为-1）
        Socket socket_; //socket对象
        std::string buffer_;    //接收缓冲区
    };
    // Reactor类
    class Reactor{
    public:
        static Reactor& getInstance();
        // 初始化
        bool init(const std::string &ip,uint16_t port,int thread_count=4);
        // 启动事件循环
        void start();
        // 停止事件循环
        void stop();
        // 处理新连接
        void handleNewConnection();
        // 处理客户端消息
        void handleClientMessage(int client_fd);
    private:
        Reactor();
        ~Reactor();
        // 禁止拷贝与赋值
        Reactor(const Reactor&) = delete;
        Reactor& operator=(const Reactor&) = delete;
        // 注册客户端连接
        void registerClientConnection(std::unique_ptr<ClientConnection> conn);
        // 移除客户端连接
        void removeClientConnection(int client_fd);
        int server_fd_;        // 服务器socketFd
        std::string server_ip;  //服务器IP
        uint16_t server_port_;  //服务器端口
        bool running_;          //运行状态
        std::unique_ptr<Epoll> epoll_;//Epoll对象
        std::unique_ptr<Socket> server_socket_;//服务器
        std::unique_ptr<ThreadPool> thread_pool_;//线程池
        //客户端连接映射
        std::unordered_map<int,std::unique_ptr<ClientConnection>> clients_;
        std::mutex client_mutex_;
        //业务模块引用
        UserManager& user_manager_;
        MessageHandler& message_handler_;
    };
}

#endif //EASYCHATSERVER_REACTOR_H
