//
// Created by Cando on 2026/1/30.
//
#include "../../include/network/reactor.h"
#include "../../include/common/protocol.h"
#include <iostream>
#include <arpa/inet.h>
#include <cstring>

namespace easychat{
    ClientConnection::ClientConnection(int fd, const std::string &ip, int port)
    :fd_(fd),ip_(ip),port_(port),user_id_(-1),socket_(fd){
        // 设置Socket为非阻塞模式
        socket_.setNonBlocking();
        std::cout<<"New client connected: "<<ip_<<":"<<port_<<", FD: "<<fd_<<std::endl;
    }
    ClientConnection::~ClientConnection() {
        std::cout<<"Client disconnected: "<<ip_<<": "<<port_<<", FD: "<<fd_<<std::endl;
    }
    void ClientConnection::handleRead() {
        char buf[4096];
        ssize_t bytes_read;
        // 读取数据（非阻塞模式）
        while ((bytes_read= socket_.recv(buf,sizeof (buf)))>0){
            buffer_.append(buf,bytes_read);
        }
        if (bytes_read==-1 && errno!=EAGAIN && errno!=EWOULDBLOCK){
            std::cerr<<"Error reading from client "<<fd_<<": "<<strerror(errno)<<std::endl;
            handleClose();
            return;
        }
        // 处理接收的数据
        while (true){
            // 检测缓冲区是否读取消息头部
            if (buffer_.size()<sizeof (MessageHeader)) break;
            // 解析消息头部
            MessageHeader header;
            std::memcpy(&header,buffer_.data(),sizeof (MessageHeader));
            // 转换字节序
            uint32_t total_length = networkTOHost32(header.length);
            uint32_t total_type = networkTOHost32(header.type);
            uint32_t total_id = networkTOHost32(header.user_id);
            // 检查缓冲区是否有完整消息
            if (buffer_.size()<total_length) break;
            // 提取消息
            Message msg=Message::deserialize(buffer_.data(),total_length);
            // 处理消息
            if (!isAuthenticated()){
                if (msg.getType()==MessageType::MSG_TYPE_LOGIN){
                    // 处理登录
                    std::string data = msg.getData();
                    size_t colon_pos = data.find(':');
                    if (colon_pos!=std::string::npos){
                        std::string username = data.substr(0,colon_pos);
                        std::string password = data.substr(colon_pos+1);
                        int login_user_id;
                        if (UserManager::getInstance().loginUser(username,password,login_user_id)){
                            // 登陆成功
                            user_id_ = login_user_id;
                            UserManager::getInstance().userOnline(user_id_,fd_,ip_,port_);
                            // 发送登录响应
                            Message resp_msg(MessageType::MSG_TYPE_LOGIN_RESP,user_id_,"Login successful");
                            sendMessage(resp_msg);
                            // 推送离线消息
                            std::vector<MessageInfo> offline_messages;
                            if (MessageHandler::getInstance().getOfflineMessage(user_id_,offline_messages)){
                                for (const auto& msg_info:offline_messages){
                                    Message offline_msg(MessageType::MSG_TYPE_OFFLINE_MSG,msg_info.sender_id,msg_info.content);
                                    sendMessage(offline_msg);
                                }
                            }else{
                                //登陆失败
                                Message resp_msg(MessageType::MSG_TYPE_ERROR,-1,"Login failed");
                                sendMessage(resp_msg);
                            }
                        }
                    }else if (msg.getType()==MessageType::MSG_TYPE_REGISTER){
                        // 处理注册
                        std::string data = msg.getData();
                        size_t colon_pos = data.find(':');
                        if (colon_pos!=std::string::npos){
                            std::string username = data.substr(0,colon_pos);
                            std::string password = data.substr(colon_pos+1);
                            if (UserManager::getInstance().registerUser(username,password)){
                                //注册成功
                                Message resp_msg(MessageType::MSG_TYPE_REGISTER_RESP,-1,"Register successful");
                                sendMessage(resp_msg);
                            }else{
                                // 注册失败
                                Message resp_msg(MessageType::MSG_TYPE_ERROR,-1,"Register failed");
                                sendMessage(resp_msg);
                            }
                        }
                    }
                }else{
                    // 已认证连接，处理其他消息
                    if (msg.getType()==MessageType::MSG_TYPE_CHAT){
                        // 处理聊天消息
                        MessageHandler::getInstance().handleReceivedMessage(msg);
                    }else if (msg.getType()==MessageType::MSG_TYPE_HEARTBEAT){
                        // 处理心跳消息
                        Message resp_msg(MessageType::MSG_TYPE_HEARTBEAT,user_id_,"Pong");
                        sendMessage(resp_msg);
                    }
                }
            }
            // 从缓存区中移除已处理的消息
            buffer_.erase(0,total_length);
        }
    }
    void ClientConnection::handleWrite() {
        // 可写事件处理
    }
    void ClientConnection::handleError() {
        std::cerr<<"Error on client "<<fd_<<std::endl;
        handleClose();
    }
    void ClientConnection::handleClose() {
        // 如果用户已认证，更新状态为离线
        if (user_id_!=-1){
            UserManager::getInstance().userOffline(user_id_);
        }
        // 关闭Socket
        socket_.close();
    }

    void ClientConnection::sendMessage(const easychat::Message &msg) {
        // 序列化消息
        auto buffer = msg.serialize();
        // 发送消息
        socket_.send(buffer.data(),buffer.size());
    }

    Reactor::Reactor()
    :server_fd_(-1),server_port_(0),running_(false),
    user_manager_(UserManager::getInstance()),
    message_handler_(MessageHandler::getInstance()){}

    Reactor::~Reactor() {stop();}

    Reactor &Reactor::getInstance() {
        static Reactor instance;
        return instance;
    }

    bool Reactor::init(const std::string &ip, uint16_t port, int thread_count) {
        server_ip = ip;
        server_port_ = port;
        // 创建服务器Socket
        server_socket_ = std::make_unique<Socket>();
        // 绑定地址与端口
        if (!server_socket_->bind(ip,port)) return false;
        // 监听连接
        if (!server_socket_->listen(128)) return false;
        // 设置非阻塞模式
        server_socket_->setNonBlocking();
        // 创建Epoll
        epoll_ = std::make_unique<Epoll>(1024);
        // 注册服务器socket到epoll
        epoll_->addFd(server_socket_->getFd(),EPOLLIN|EPOLLET);
        // 设置服务器Socket的回调函数
        epoll_->setReadCallback(server_socket_->getFd(),[this]{this->handleNewConnection();});
        // 创建线程池
        thread_pool_ = std::make_unique<ThreadPool>(thread_count);
        server_fd_ = server_socket_->getFd();
        running_ = true;

        std::cout<<"Reactor initialized, listening on "<<ip<<":"<<port<<std::endl;
        return true;
    }

    void Reactor::start() {
        std::cout<<"Reactor starting..."<<std::endl;
        while (running_){
            // 等待事件
            int num_events = epoll_->wait(1000);
            if (num_events==-1){
                if (errno!=EINTR){
                    std::cerr<<"Epoll wait error:"<<strerror(errno)<<std::endl;
                    break;
                }
                continue;
            }
            // 处理事件
            epoll_->handleEvents(num_events);
        }
        std::cout<<"Reactor stopped"<<std::endl;
    }

    void Reactor::stop() {
        running_ = false;
        // 关闭所有客户端连接
        for (auto& [fd,conn]:clients_){
            conn->handleClose();
        }
        clients_.clear();
        // 关闭服务器Socket
        if (server_socket_){
            server_socket_->close();
        }
    }

    void Reactor::handleNewConnection() {
        // 接收新连接
        while (true){
            auto client_socket = server_socket_->accept();
            if (!client_socket){
                if (errno==EAGAIN || errno==EWOULDBLOCK){
                    // 没有更多连接
                    break;
                }else{
                    std::cerr<<"Error accepting connection: "<<strerror(errno)<<std::endl;
                    break;
                }
            }
            // 获取客户端地址
            sockaddr_in client_addr;
            socklen_t client_len = sizeof (client_addr);
            getpeername(client_socket->getFd(),(sockaddr*)&client_addr,&client_len);

            std::string client_ip = inet_ntoa(client_addr.sin_addr);
            int client_port = ntohs(client_addr.sin_port);

            // 创建客户端连接
            auto conn = std::make_unique<ClientConnection>(
                    client_socket->getFd(),client_ip,client_port
                    );
            // 注册客户端连接
            registerClientConnection(std::move(conn));
        }
    }

    void Reactor::registerClientConnection(std::unique_ptr<ClientConnection> conn) {
        int client_fd = conn->getFd();
        // 添加到Epoll
        epoll_->addFd(client_fd,EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP);
        // 设置回调函数
        epoll_->setReadCallback(client_fd,
    [this,client_fd]{thread_pool_->submit([this,client_fd]{this->handleClientMessage(client_fd);});
        });
        epoll_->setErrorCallback(client_fd,[conn=conn.get()]{conn->handleError();});
        epoll_->setCloseCallback(client_fd,[conn=conn.get()]{conn->handleClose();});
        // 添加到客户端映射
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            clients_[client_fd] = std::move(conn);
        }

        std::cout<<"Client connection registered: FD="<<client_fd<<std::endl;
    }

    void Reactor::removeClientConnection(int client_fd) {
        // 从Epoll中移除
        epoll_->removeFd(client_fd);
        // 从客户端映射中移除
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            clients_.erase(client_fd);
        }
        std::cout<<"Client connection removed: FD="<<client_fd<<std::endl;
    }

    void Reactor::handleClientMessage(int client_fd) {
        {
            std::lock_guard<std::mutex> lock(client_mutex_);
            auto it = clients_.find(client_fd);
            if (it!=clients_.end()){
                it->second->handleRead();
            }
        }
    }
}
