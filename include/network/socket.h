//
// Created by 30501 on 2026/1/29.
//

#ifndef EASYCHATSERVER_SOCKET_H
#define EASYCHATSERVER_SOCKET_H

#include<string>
#include <memory>

namespace easychat{
    // socket封装类
    class Socket{
    public:
        Socket();
        explicit Socket(int fd, bool owns_fd = true);
        ~Socket();

        // 禁止拷贝，允许移动
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;

        // 绑定地址和端口
        bool bind(const std::string& ip,uint16_t port);
        // 监听连接
        bool listen(int backlog = 128);
        // 接受连接
        std::unique_ptr<Socket> accept();
        // 连接到服务器
        bool connect(const std::string& ip,uint16_t port);
        // 接收数据
        ssize_t recv(char* buffer,size_t length);
        // 发送数据
        ssize_t send(const char* data,size_t length);
        // 设置为非阻塞模式
        bool setNonBlocking();
        // 关闭close();
        void close();
        // 获取文件描述符
        int getFd() const {return fd_; }
        // 获取Socket错误
        int getSocketError() const;
        // 释放文件描述符所有权
        void releaseOwnership() { owns_fd_ = false; }
    private:
        int fd_;
        bool owns_fd_;  // 是否拥有文件描述符
    };
}

#endif //EASYCHATSERVER_SOCKET_H
