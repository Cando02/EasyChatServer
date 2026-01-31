//
// Created by Cando on 2026/1/31.
//
#include "../../include/common/signal_handler.h"
#include <iostream>
#include <cstring>

namespace easychat{
    bool SignalHandler::shutdown_flag_= false;

    SignalHandler &SignalHandler::getInstance() {
        static SignalHandler instance;
        return instance;
    }

    void SignalHandler::registerHandler(int signal, std::function<void()> handler) {
        handlers_[signal] = handler;
    }

    void SignalHandler::init() {
        //注册信号处理函数
        signal(SIGINT,SignalHandler::signalHandlerFunc);
        signal(SIGTERM,SignalHandler::signalHandlerFunc);
        signal(SIGQUIT,SignalHandler::signalHandlerFunc);
        signal(SIGPIPE,SIG_IGN);

        std::cout<<"Signal handlers initialized"<<std::endl;
    }

    void SignalHandler::signalHandlerFunc(int signal) {
        std::cout<<"Received signal: "<<signal<<" ("<<strsignal(signal)<<")"<<std::endl;
        // 调用注册的处理函数
        auto& instance = SignalHandler::getInstance();
        auto it = instance.handlers_.find(signal);
        if (it!=instance.handlers_.end()){
            it->second();
        }
        // 设置关闭标志
        shutdown_flag_= true;
    }

}