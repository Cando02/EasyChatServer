//
// Created by Cando on 2026/1/29.
//
#include "../../include/network/epoll.h"
#include <unistd.h>
#include <cstring>
#include <iostream>

namespace easychat{
    Epoll::Epoll(int max_events):
    max_events_(max_events),events_(nullptr){
        // 创建 epoll 实例
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_==-1){
            std::cerr<<"Failed to create epoll instance: "<<
            strerror(errno)<<std::endl;
            exit(EXIT_FAILURE);
        }
        // 分配事件数组内存
        events_ = new struct epoll_event[max_events_];
        std::memset(events_,0,sizeof (struct epoll_event)*max_events_);
    }
    Epoll::~Epoll(){
        // 关闭 epoll 文件描述符
        if (epoll_fd_ != -1){
            close(epoll_fd_);
        }
        // 释放事件数组内存
        if (events_!= nullptr){
            delete[] events_;
        }
    }
    bool Epoll::addFd(int fd, uint32_t events) {
        // 设置 epoll 事件
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;

        // 将"添加"文件描述符添加到epoll实例中
        if (epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,fd,&ev)==-1){
            std::cerr<<"Failed to add fd to epoll: "<<strerror(errno)<<std::endl;
            return false;
        }

        return true;
    }
    bool Epoll::modifyFd(int fd, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        // 将"修改"文件描述符添加到epoll实例中
        if (epoll_ctl(epoll_fd_,EPOLL_CTL_MOD,fd,&ev)==-1){
            std::cerr<<"Failed to modify fd in epoll: "<<strerror(errno)<<std::endl;
            return false;
        }
        return true;
    }
    bool Epoll::removeFd(int fd) {
        // 将"删除"文件描述符添加到epoll实例中
        if (epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,fd, nullptr)==-1){
            std::cerr<<"Failed to remove fd from epoll: "<<strerror(errno)<<std::endl;
            return false;
        }
        // 清除回调函数
        read_callbacks_.erase(fd);
        write_callbacks_.erase(fd);
        error_callbacks_.erase(fd);
        close_callbacks_.erase(fd);

        return true;
    }
    int Epoll::wait(int timeout_ms){
        //等待事件发生
        int num_events = epoll_wait(epoll_fd_,events_,max_events_,timeout_ms);
        if (num_events==-1){
            // EINTR 表示被信号中断，不是真正的错误
            if (errno!=EINTR){
                std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
            }
        }
        return num_events;
    }
    int Epoll::getReadyFd(int index) const {
        if (index<0||index>=max_events_){
            return -1;
        }
        return events_[index].data.fd;
    }
    uint32_t Epoll::getReadyEvents(int index) const {
        if (index<0 || index>= max_events_){
            return 0;
        }
        return events_[index].events;
    }
    void Epoll::setReadCallback(int fd, EventCallback cb) {
        read_callbacks_[fd] = cb;
    }
    void Epoll::setWriteCallback(int fd, EventCallback cb) {
        write_callbacks_[fd] = cb;
    }
    void Epoll::setErrorCallback(int fd, EventCallback cb) {
        error_callbacks_[fd] = cb;
    }
    void Epoll::setCloseCallback(int fd, EventCallback cb) {
        close_callbacks_[fd] = cb;
    }
    void Epoll::handleEvents(int num_events) {
        //遍历就绪事件
        for (int i=0;i<num_events;++i){
            int fd = getReadyFd(i);
            uint32_t events = getReadyEvents(i);

            // 处理错误事件
            if (events & (EPOLLERR | EPOLLHUP)){
                auto it = error_callbacks_.find(fd);
                if (it!=error_callbacks_.end() && it->second){
                    it->second();
                }
                continue;
            }
            //处理可读事件
            if (events & EPOLLIN){
                auto it = read_callbacks_.find(fd);
                if (it!=read_callbacks_.end() && it->second){
                    it->second();
                }
            }
            // 处理可写事件
            if (events & EPOLLOUT){
                auto it = write_callbacks_.find(fd);
                if (it!=write_callbacks_.end() && it->second){
                    it->second();
                }
            }
        }
    }
}