//
// Created by Cando on 2026/1/31.
//

#ifndef EASYCHATSERVER_SIGNAL_HANDLER_H
#define EASYCHATSERVER_SIGNAL_HANDLER_H

#include <functional>
#include <unordered_map>
#include <csignal>

namespace easychat{
    // 信号处理类-单例模式
    class SignalHandler{
    public:
        static SignalHandler& getInstance();
        // 注册信号处理函数
        void registerHandler(int signal,std::function<void()> handler);
        // 初始化信号处理
        void init();
        // 设置优雅关闭标志
        void setShutdownFlag(bool flag){
            shutdown_flag_=flag;
        }
        // 检查是否需要关闭
        bool shouldShutdown() const{
            return shutdown_flag_;
        }
    private:
        SignalHandler()=default;
        ~SignalHandler()=default;
        // 禁止拷贝与赋值
        SignalHandler(const SignalHandler&)=delete;
        SignalHandler& operator=(const SignalHandler&)=delete;
        // 信号处理函数
        static void signalHandlerFunc(int signal);
        // 信号处理器映射
        std::unordered_map<int,std::function<void()>> handlers_;
        // 优雅关闭标志
        static bool shutdown_flag_;
    };
}
#endif //EASYCHATSERVER_SIGNAL_HANDLER_H
