//
// Created by Cando on 2026/1/29.
//
#include "../../include/database/connection_pool.h"
#include <iostream>
namespace easychat {
    MySQLConnection::MySQLConnection() : mysql_(nullptr),connected_(false){
        //初始化MySQL连接对象
        mysql_ = mysql_init(nullptr);
        if (mysql_== nullptr){
            std::cerr<<"Failed to initialize MySQL connection"<<std::endl;
        }
    }
    MySQLConnection::~MySQLConnection() {
        close();
    }
    bool MySQLConnection::connect(const std::string &host, uint16_t port, const std::string &user,
                                  const std::string &password, const std::string &database) {
        if (mysql_== nullptr) return false;
        // 连接到mysql服务器
        MYSQL* result = mysql_real_connect(
                mysql_,
                host.c_str(),
                user.c_str(),
                password.c_str(),
                database.c_str(),
                port,
                nullptr,
                0
                );
        if (result== nullptr){
            std::cerr<<"Failed to connect to MySQL: "<<mysql_error(mysql_)<<std::endl;
            return false;
        }
        // 设置字符集为UTF-8
        if (mysql_set_character_set(mysql_,"utf8mb4")!=0){
            std::cerr<<"Failed to set character set:"<<mysql_error(mysql_)<<std::endl;
            return false;
        }

        connected_ = true;
        std::cout<<"MySQL connection established"<<std::endl;
        return true;
    }
    bool MySQLConnection::execute(const std::string &sql) {
        if (!connected_ || mysql_ == nullptr) return false;
        // 执行SQL语句
        if (mysql_query(mysql_,sql.c_str())!=0){
            std::cerr<<"Failed to execute SQL: "<<mysql_error(mysql_)<<std::endl;
            return false;
        }
        return true;
    }

    MYSQL_RES *MySQLConnection::query(const std::string &sql) {
        if (!execute(sql)) return nullptr;
        // 获取查询结果
        MYSQL_RES* result = mysql_store_result(mysql_);
        if (result== nullptr){
            std::cerr<<"Failed to get query result: "<<mysql_error(mysql_)<<std::endl;
        }
        return result;
    }
    void MySQLConnection::close() {
        // 关闭MySQL连接
        if (mysql_!= nullptr){
            mysql_close(mysql_);
            mysql_ = nullptr;
        }
        connected_ = false;
    }
    ConnectionPool::ConnectionPool() :port_(3306),pool_size_(10),initialized_(false){}
    ConnectionPool::~ConnectionPool() {close();}

    ConnectionPool &ConnectionPool::getInstance() {
        static ConnectionPool instance;
        return instance;
    }

    bool
    ConnectionPool::init(const std::string &host, uint16_t port, const std::string &user, const std::string &password,
                         const std::string &database, size_t pool_size) {
        if (initialized_){
            std::cout << "Connection pool already initialized" << std::endl;
            return true;
        }
        host_ = host;
        port_ = port;
        user_ = user;
        password_ = password;
        database_ = database;
        pool_size_ = pool_size;
        // 创建指定数量的连接
        for (size_t i=0;i<pool_size;++i){
            auto conn = std::make_shared<MySQLConnection>();
            if (!conn->connect(host_,port_,user_,password_,database_)){
                std::cout << "Failed to create connection "<< i << std::endl;
                continue;
            }
            // 将连接添加到队列
            connections_.push(conn);
        }
        if (connections_.size()==0){
            std::cerr<<"Failed to initialize connection pool"<<std::endl;
            return false;
        }
        initialized_ = true;
        std::cout << "Connection pool initialized with"<< connections_.size()<<" connections" << std::endl;
        return true;
    }

    std::shared_ptr<MySQLConnection> ConnectionPool::getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        // 等待可用连接
        condition_.wait(lock,[this]{
            return !this->connections_.empty() || !this->initialized_;
        });
        if (!initialized_){
            return nullptr;
        }
        // 从队列中取出连接
        auto conn = connections_.front();
        connections_.pop();

        return conn;
    }

    void ConnectionPool::returnConnection(std::shared_ptr<MySQLConnection> conn) {
        if (conn== nullptr) return;
        std::lock_guard<std::mutex> lock(mutex_);
        // 将连接归还队列
        connections_.push(conn);
        // 通知等待线程
        condition_.notify_one();
    }

    void ConnectionPool::close() {
        std::lock_guard<std::mutex> lock(mutex_);
        initialized_ = false;
        // 清空队列
        while (!connections_.empty()) connections_.pop();
        //通知所有等等线程
        condition_.notify_all();
        std::cout << "Connection pool closed" << std::endl;
    }
}