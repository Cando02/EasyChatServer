#include <iostream>
#include <signal.h>
#include "network/reactor.h"
#include "database/connection_pool.h"
#include "business/user_manager.h"
#include "business/message_handler.h"

using namespace easychat;

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "Received signal " << signum << ", stopping server..." << std::endl;
    Reactor::getInstance().stop();
    exit(signum);
}

int main() {
    std::cout << "=== EasyChatServer ===" << std::endl;

    // 注册信号处理
    signal(SIGINT, signalHandler);    // Ctrl+C
    signal(SIGTERM, signalHandler);   // 终止信号

    // 初始化数据库连接池
    std::cout << "Initializing database connection pool..." << std::endl;
    auto& conn_pool = ConnectionPool::getInstance();
    if (!conn_pool.init("localhost", 3306, "root", "123456", "easychat", 10)) {
        std::cerr << "Failed to initialize database connection pool" << std::endl;
        return 1;
    }

    // 初始化业务模块
    std::cout << "Initializing business modules..." << std::endl;
    UserManager::getInstance().init();
    MessageHandler::getInstance().init();

    // 初始化 Reactor
    std::cout << "Initializing reactor..." << std::endl;
    if (!Reactor::getInstance().init("0.0.0.0", 8888, 4)) {
        std::cerr << "Failed to initialize reactor" << std::endl;
        return 1;
    }

    // 启动服务器
    std::cout << "Starting server..." << std::endl;
    Reactor::getInstance().start();

    return 0;
}