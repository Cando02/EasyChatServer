//
// Created by 30501 on 2026/1/29.
//

#ifndef EASYCHATSERVER_EPOLL_H
#define EASYCHATSERVER_EPOLL_H

#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <unordered_map>

namespace easychat{

    // 前向声明,通讯端点,嵌套字
    class Socket;
    using EventCallback = std::function<void()>;

    // Epoll 事件类型
    enum class EpollEventType : uint32_t {
        READ = EPOLLIN, // 可读事件
        WRITE = EPOLLOUT, // 可写事件
        ERROR = EPOLLERR, // 错误事件
        HANGUP = EPOLLHUP,// 对方关闭连接
        EDGE_TRIGGERED = EPOLLET// 边缘触发模式
    };
    // Epoll 封装类
    class Epoll{
    public:
        explicit Epoll(int max_vents = 1024);
        ~Epoll();

        // 禁止拷贝和赋值
        Epoll(const Epoll&) = delete;
        Epoll& operator=(const Epoll&) = delete;

        // 添加文件描述符到 epoll 实例
        bool addFd(int fd,uint32_t events);

        // 修改文件描述符的事件
        bool modifyFd(int fd,uint32_t events);

        // 从epoll实例中删除文件描述符
        bool removeFd(int fd);

        // 等待事件发生，-1为无限等待，0为立即返回
        int wait(int timeout_ms=-1);

        // 就绪文件描述符
        int getReadyFd(int index) const;

        // 获取就绪事件
        uint32_t getReadyEvents(int index) const;

        // 设置文件描述符的回调函数
        void setReadCallback(int fd,EventCallback cb);
        void setWriteCallback(int fd,EventCallback cb);
        void setErrorCallback(int fd,EventCallback cb);
        void setCloseCallback(int fd,EventCallback cb);

        // 处理就绪事件
        void handleEvents(int num_events);
    private:
        int epoll_fd_;
        int max_events_;
        struct epoll_event* events_;

        //回调函数映射表
        std::unordered_map<int,EventCallback> read_callbacks_;
        std::unordered_map<int,EventCallback> write_callbacks_;
        std::unordered_map<int,EventCallback> error_callbacks_;
        std::unordered_map<int,EventCallback> close_callbacks_;
    };
}

#endif //EASYCHATSERVER_EPOLL_H
