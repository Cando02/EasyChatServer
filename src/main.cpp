#include <iostream>
#include "common/config.h"
#include "common/logger.h"
#include "common/daemon.h"
#include "common/signal_handler.h"
#include "network/reactor.h"
#include "database/connection_pool.h"
#include "business/user_manager.h"
#include "business/message_handler.h"

using namespace easychat;

int main(int argc, char* argv[]) {
    std::cout << "=== EasyChatServer ===" << std::endl;

    // 解析命令行参数
    bool daemon_mode = false;
    std::string config_file = "config/server.conf";

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-d" || std::string(argv[i]) == "--daemon") {
            daemon_mode = true;
        } else if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config") {
            if (i + 1 < argc) {
                config_file = argv[i + 1];
                ++i;
            }
        } else if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -d, --daemon    Run as daemon" << std::endl;
            std::cout << "  -c, --config    Config file path (default: config/server.conf)" << std::endl;
            std::cout << "  -h, --help      Show this help message" << std::endl;
            return 0;
        }
    }

    // 加载配置文件
    std::cout << "Loading config file: " << config_file << std::endl;
    if (!Config::getInstance().load(config_file)) {
        std::cerr << "Failed to load config file: " << config_file << std::endl;
        return 1;
    }
    std::cout << "Config loaded successfully" << std::endl;

    // 初始化日志系统
    std::cout << "Initializing logger..." << std::endl;
    std::string log_level_str = Config::getInstance().getString("log.level", "INFO");
    LogLevel log_level = LogLevel::INFO;
    if (log_level_str == "DEBUG") log_level = LogLevel::LOG_DEBUG;
    else if (log_level_str == "INFO") log_level = LogLevel::INFO;
    else if (log_level_str == "WARN") log_level = LogLevel::WARN;
    else if (log_level_str == "ERROR") log_level = LogLevel::ERROR;

    std::string log_file = Config::getInstance().getString("log.file_path", "logs/server.log");
    bool log_console = Config::getInstance().getBool("log.console", true);
    size_t log_max_size = Config::getInstance().getInt("log.max_size", 100) * 1024 * 1024;
    int log_backup_count = Config::getInstance().getInt("log.backup_count", 5);

    std::cout << "Log file: " << log_file << std::endl;
    std::cout << "Log level: " << log_level_str << std::endl;

    if (!Logger::getInstance().init(log_file, log_level, log_console, log_max_size, log_backup_count)) {
        std::cerr << "Failed to initialize logger" << std::endl;
        return 1;
    }
    std::cout << "Logger initialized successfully" << std::endl;

    LOG_INFO()<<"EasyChatServer starting...";
    LOG_INFO()<<"Config file: " + config_file;

    // 启动守护进程（如果需要）
    if (daemon_mode) {
        LOG_INFO()<<"Starting in daemon mode...";
        if (!Daemon::start()) {
            LOG_ERROR()<<"Failed to start daemon";
            return 1;
        }

        // 写入 PID 文件
        std::string pid_file = "EasyChatServer.pid";
        if (!Daemon::writePidFile(pid_file)) {
            LOG_ERROR()<<"Failed to write PID file";
            return 1;
        }
    }

    // 初始化信号处理
    LOG_INFO()<<"Initializing signal handlers...";
    auto& signal_handler = SignalHandler::getInstance();
    signal_handler.registerHandler(SIGINT, []() {
        LOG_INFO()<<"SIGINT received, shutting down gracefully...";
    });
    signal_handler.registerHandler(SIGTERM, []() {
        LOG_INFO()<<"SIGTERM received, shutting down gracefully...";
    });
    signal_handler.init();
    LOG_INFO()<<"Signal handlers initialized successfully";

    // 初始化数据库连接池
    LOG_INFO()<<"Initializing database connection pool...";
    std::string db_host = Config::getInstance().getString("database.host", "localhost");
    uint16_t db_port = Config::getInstance().getPort("database.port", 3306);
    std::string db_user = Config::getInstance().getString("database.user", "easychat");
    std::string db_password = Config::getInstance().getString("database.password", "password");
    std::string db_name = Config::getInstance().getString("database.database", "easychat");
    int db_pool_size = Config::getInstance().getInt("database.max_connections", 10);

    LOG_INFO()<<"Database config: "<<db_host<<":"<<std::to_string(db_port)<<", user: "<<db_user<<", db: "<<db_name;

    auto& conn_pool = ConnectionPool::getInstance();
    if (!conn_pool.init(db_host, db_port, db_user, db_password, db_name, db_pool_size)) {
        LOG_ERROR()<<"Failed to initialize database connection pool";
        std::cerr << "Failed to initialize database connection pool" << std::endl;
        return 1;
    }
    LOG_INFO()<<"Database connection pool initialized successfully";

    // 初始化业务模块
    LOG_INFO()<<"Initializing business modules...";
    UserManager::getInstance().init();
    MessageHandler::getInstance().init();
    LOG_INFO()<<"Business modules initialized successfully";

    // 初始化 Reactor
    LOG_INFO()<<"Initializing reactor...";
    std::string server_host = Config::getInstance().getString("server.host", "0.0.0.0");
    uint16_t server_port = Config::getInstance().getPort("server.port", 8888);
    int thread_pool_size = Config::getInstance().getInt("server.thread_pool_size", 4);

    LOG_INFO()<<"Server config: " + server_host + ":" + std::to_string(server_port) + ", thread pool size: " + std::to_string(thread_pool_size);

    if (!Reactor::getInstance().init(server_host, server_port, thread_pool_size)) {
        LOG_ERROR()<<"Failed to initialize reactor";
        std::cerr << "Failed to initialize reactor" << std::endl;
        return 1;
    }
    LOG_INFO()<<"Reactor initialized successfully";

    // 启动服务器
    LOG_INFO()<<"Starting server on " + server_host + ":" + std::to_string(server_port);
    std::cout << "Server starting on " << server_host << ":" << server_port << std::endl;
    Reactor::getInstance().start();

    // 清理资源
    LOG_INFO()<<"Shutting down...";
    std::cout << "Server shutting down..." << std::endl;
    Logger::getInstance().close();

    // 删除 PID 文件（如果是守护进程）
    if (daemon_mode) {
        Daemon::removePidFile("EasyChatServer.pid");
    }

    LOG_INFO()<<"EasyChatServer stopped";
    std::cout << "Server stopped" << std::endl;
    return 0;
}