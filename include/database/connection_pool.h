//
// Created by Cando on 2026/1/29.
//

#ifndef EASYCHATSERVER_CONNECTION_POOL_H
#define EASYCHATSERVER_CONNECTION_POOL_H

#include <mysql/mysql.h>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>

namespace easychat{
    // mysql连接封装类
    class MySQLConnection{
    public:
        MySQLConnection();
        ~MySQLConnection();
        // 连接数据库
        bool connect(const std::string& host,
                     uint16_t port,
                     const std::string& user,
                     const std::string& password,
                     const std::string& database);
        // 执行sql语言
        bool execute(const std::string& sql);
        // 查询数据
        MYSQL_RES* query(const std::string& sql);
        // 获取连接对象
        MYSQL* getMySQL(){return mysql_;}
        //检查连接是否有效
        bool isConnected() const {return connected_;}
        // 关闭连接
        void close();
    private:
        MYSQL * mysql_;
        bool connected_;
    };
    // 数据库连接池类
    class ConnectionPool{
    public:
        static ConnectionPool& getInstance();
        //初始化连接池
        bool init(const std::string& host,
                  uint16_t port,
                  const std::string& user,
                  const std::string& password,
                  const std::string& database,
                  size_t pool_size);
        // 获取连接
        std::shared_ptr<MySQLConnection> getConnection();
        // 归还连接
        void returnConnection(std::shared_ptr<MySQLConnection> conn);
        // 获取连接池大小
        size_t getPoolSize() const {
            return pool_size_;
        }
        // 获取空闲连接数
        size_t getAvailableConnections() const{
            std::lock_guard<std::mutex> lock(mutex_);
            return connections_.size();
        }
        // 关闭连接池
        void close();
    private:
        ConnectionPool();
        ~ConnectionPool();
        //禁止拷贝和赋值
        ConnectionPool(const ConnectionPool&) = delete;
        ConnectionPool& operator=(const ConnectionPool&) = delete;
        // 连接队列
        std::queue<std::shared_ptr<MySQLConnection>> connections_;
        // 互斥锁
        mutable std::mutex mutex_;
        // 条件变量
        std::condition_variable condition_;
        std::string host_;
        uint16_t port_;
        std::string user_;
        std::string password_;
        std::string database_;
        size_t pool_size_;
        std::atomic<bool> initialized_;
    };
}

#endif //EASYCHATSERVER_CONNECTION_POOL_H
