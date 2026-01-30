//
// Created by Cando on 2026/1/29.
//
#include "../../include/network/socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>

namespace easychat{
    Socket::Socket() : fd_(-1){
        // 创建Socket
        // AF_INET: IPv4 地址族
        // SOCK_STREAM: 面向连接的 TCP 套接字
        // 0: 使用默认协议
        // ::socket表示使用全局的socket
        fd_ = ::socket(AF_INET,SOCK_STREAM,0);
        if (fd_==-1){
            std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        }
    }
    Socket::Socket(int fd) : fd_(fd) {}
    Socket::~Socket(){close();}
    Socket::Socket(Socket&& other) noexcept : fd_(other.fd_){
        other.fd_=-1;
    }

    Socket &Socket::operator=(easychat::Socket &&other) noexcept {
        if (this!=&other){
            close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    bool Socket::bind(const std::string &ip, uint16_t port) {
        if (fd_==-1){return false;}
        // 设置地址重用选项
        int opt = 1;
        if (::setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))==-1){
            std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
            return false;
        }
        //绑定地址与端口
        struct sockaddr_in addr;
        std::memset(&addr,0,sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());

        if (::bind(fd_,(struct  sockaddr*)&addr,sizeof(addr))==-1){
            std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    bool Socket::listen(int backlog) {
        if (fd_==-1){
            return false;
        }
        // 开始监听
        if (::listen(fd_,backlog)==-1){
            std::cerr<<"Failed to listen socket: "<<strerror(errno)<<std::endl;
            return false;
        }
        return true;
    }
    std::unique_ptr<Socket> Socket::accept() {
        if (fd_==-1){
            return nullptr;
        }
        // 接收连接
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof (client_addr);

        int client_fd = ::accept(fd_,(struct sockaddr*)&client_addr,&client_len);
        if (client_fd==-1){
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            }
            return nullptr;
        }
        // 打印客户端信息
        std::cout<<"New connection from "<<inet_ntoa(client_addr.sin_addr)
        <<":"<<ntohs(client_addr.sin_port)<<std::endl;
        return std::make_unique<Socket>(client_fd);
    }
    bool Socket::connect(const std::string &ip, uint16_t port) {
        if (fd_==-1){return false;}
        struct sockaddr_in server_addr;
        std::memset(&server_addr,0,sizeof (server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

        if (::connect(fd_,(struct sockaddr*)&server_addr,sizeof (server_addr))==-1){
            std::cerr<<"Failed to connect: "<<strerror(errno)<<std::endl;
            return false;
        }
        return true;
    }
    ssize_t Socket::recv(char *buffer, size_t length) {
        if (fd_==-1) return -1;
        // 接收数据
        // recv()返回接收字节数，0为连接关闭，-1为出错
        ssize_t bytes = ::recv(fd_,buffer,length,0);
        if (bytes==-1){
            if (errno!=EAGAIN && errno!=EWOULDBLOCK){
                std::cerr<<"Failed to recv data: "<<strerror(errno)<<std::endl;
            }
        }else if (bytes==0){
            std::cout<<"Connection closed by peer"<<std::endl;
        }
        return bytes;
    }
    ssize_t Socket::send(const char *data, size_t length) {
        if (fd_==-1) return -1;
        // 发送数据
        ssize_t bytes = ::send(fd_,data,length,0);
        if (bytes==-1){
            if (errno!=EAGAIN && errno!=EWOULDBLOCK){
                std::cerr<<"Failed to send data: "<<strerror(errno)<<std::endl;
            }
        }
        return bytes;
    }
    bool Socket::setNonBlocking() {
        if (fd_==-1) return false;
        // 获取文件描述符
        int flags = fcntl(fd_,F_GETFL,0);
        if (flags==-1){
            std::cerr<<"Failed to get socket flags: "<<strerror(errno)<<std::endl;
            return false;
        }
        // 设置非阻塞标志
        if (fcntl(fd_,F_SETFL,flags | O_NONBLOCK)==-1){
            std::cerr<<"Failed to set non-blocking: "<<strerror(errno)<<std::endl;
            return false;
        }
        return true;
    }
    void Socket::close() {
        if (fd_==-1){
            ::close(fd_);
            fd_ = -1;
        }
    }
    int Socket::getSocketError() const {
        if (fd_==-1) return -1;
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(fd_,SOL_SOCKET,SO_ERROR,&error,&len)==-1) {
            return errno;
        }
        return error;
    }
}