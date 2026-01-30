//
// Created by Cando on 2026/1/30.
//
#include "../../include/business/user_manager.h"
#include <mysql/mysql.h>
#include <cstring>
#include <iostream>
#include <openssl/md5.h>
#include <iomanip>
#include <sstream>

namespace easychat{
    UserManager::UserManager() : conn_pool_(ConnectionPool::getInstance()){}
    UserManager::~UserManager(){}

    UserManager &UserManager::getInstance() {
        static UserManager instance;
        return instance;
    }
    void UserManager::init(){
        std::cout<<"UserManager initialized"<<std::endl;
    }

    std::string UserManager::encryptPassword(const std::string &password) {
        // 使用OpenSSL的MD5函数对密码加密
        unsigned char md5_hash[MD5_DIGEST_LENGTH];
        MD5(reinterpret_cast<const unsigned char*>(password.c_str()),password.length(),md5_hash);
        // 将MD5哈希转化为十六进制字符串
        std::stringstream ss;
        for (int i=0;i<MD5_DIGEST_LENGTH;++i){
            ss<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<int>(md5_hash[i]);
        }
        return ss.str();
    }
    bool UserManager::registerUser(const std::string &username, const std::string &password,
                                  const std::string &nickname) {
        // 获取数据库连接
        auto conn = conn_pool_.getConnection();
        if (!conn||!conn->isConnected()){
            std::cerr<<"Failed to get database connection"<<std::endl;
            return false;
        }
        // 检查用户名是否存在
        std::string check_sql = "select id from users where username='"+username+"'";
        MYSQL_RES* result = conn->query(check_sql);
        if (result && mysql_num_rows(result)>0){
            mysql_free_result(result);
            std::cerr<<"Username already exists: "<<username<<std::endl;
            return false;
        }
        if (result){
            mysql_free_result(result);
        }
        // 加密密码
        std::string encrypted_pwd = encryptPassword(password);
        // 插入新用户
        std::string insert_sql = "insert into user (username,password,nickname) values('"+username+"','"+encrypted_pwd+"','"+nickname+"'";
        if (!conn->execute(insert_sql)){
            std::cerr<<"Failed to register user: "<<username<<std::endl;
            return false;
        }
        std::cout<<"User registered successfully: "<<username<<std::endl;
        return true;
    }

    bool UserManager::loginUser(const std::string &username, const std::string &password, int &user_id) {
        // 获取数据库连接
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()){
            std::cerr<<"Failed to get database connection"<<std::endl;
            return false;
        }
        // 加密密码
        std::string encrypted_pwd = encryptPassword(password);
        // 查询用户
        std::string query_sql = "select id from users where username='"+username+"' and password='"+encrypted_pwd+"'";
        MYSQL_RES* result = conn->query(query_sql);
        if (!result || mysql_num_rows(result)==0){
            if (result){
                mysql_free_result(result);
            }
            std::cerr<<"Login failed: invalid username or password"<<std::endl;
            return false;
        }
        // 获取用户ID
        MYSQL_ROW row = mysql_fetch_row(result);
        user_id = std::stoi(row[0]);
        mysql_free_result(result);
        // 更新用户状态为在线
        updateUserStatus(user_id, 1);
        std::cout<<"User logged in successfully: "<<username<<"(ID:"<<user_id<<")"<<std::endl;
        return true;
    }
    bool UserManager::getUserInfo(int user_id, easychat::UserInfo &user_info) {
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;
        std::string query_sql = "select id,username,password,nickname,avatar,status from users where id="+std::to_string(user_id);
        MYSQL_RES *result = conn->query(query_sql);
        if (!result|| mysql_num_rows(result)==0){
            if (result) mysql_free_result(result);
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(result);
        user_info.id = std::stoi(row[0]);
        user_info.username = row[1];
        user_info.password = row[2];
        user_info.nickname = row[3] ? row[3] : "";
        user_info.avatar = row[4] ? row[4] : "";
        user_info.status = std::stoi(row[5]);
        mysql_free_result(result);
        return true;
    }

    bool UserManager::getUserInfo(const std::string &username, easychat::UserInfo &user_info) {
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;
        std::string query_sql = "select id,username,password,nickname,avatar,status from users where username='"+username+"'";
        MYSQL_RES *result = conn->query(query_sql);
        if (!result|| mysql_num_rows(result)==0){
            if (result) mysql_free_result(result);
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(result);
        user_info.id = std::stoi(row[0]);
        user_info.username = row[1];
        user_info.password = row[2];
        user_info.nickname = row[3] ? row[3] : "";
        user_info.avatar = row[4] ? row[4] : "";
        user_info.status = std::stoi(row[5]);
        mysql_free_result(result);
        return true;
    }

    bool UserManager::updateUserStatus(int user_id, int status) {
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;
        std::string update_sql = "update users set status="+std::to_string(status)+" where id="+std::to_string(user_id);
        if (!conn->execute(update_sql)){
            return false;
        }
        return true;
    }

    bool UserManager::userOnline(int user_id, int socket_fd, const std::string &ip, int port) {
        // 更新数据库状态
        if (!updateUserStatus(user_id,1)) return false;
        // 更新在线用户映射
        {
            std::lock_guard<std::mutex> lock(online_user_mutex_);
            online_users_[user_id] = socket_fd;
        }
        //将用户添加到在线用户表
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;

        std::string insert_sql = "insert ignore into online_users(user_id,socket_fd,ip,port) values("
                +std::to_string(user_id)+", "+std::to_string(socket_fd)+", "+ip+", "+std::to_string(port)+")";
        conn->execute(insert_sql);
        std::cout<<"User online: ID="<<user_id<<", SocketFd="<<socket_fd<<std::endl;
        return true;
    }

    bool UserManager::userOffline(int user_id) {
        // 更新数据库状态
        if (!updateUserStatus(user_id,0)) return false;
        // 从在线用户映射中移除
        {
            std::lock_guard<std::mutex> lock(online_user_mutex_);
            online_users_.erase(user_id);
        }
        // 从在线用户表中移除
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return false;
        std::string delete_sql = "delete from online_users where user_id = "+std::to_string(user_id);
        conn->execute(delete_sql);

        std::cout<<"User offline ID="<<user_id<<std::endl;
        return true;
    }
    bool UserManager::isUserOnline(int user_id) {
        std::lock_guard<std::mutex> lock(online_user_mutex_);
        return online_users_.find(user_id)!=online_users_.end();
    }

    std::unordered_map<int, UserInfo> UserManager::getOnlineUsers() {
        std::unordered_map<int,UserInfo> online_users;
        auto conn = conn_pool_.getConnection();
        if (!conn || !conn->isConnected()) return online_users;

        std::string query_sql = "select id from online_users";
        MYSQL_RES *result = conn->query(query_sql);
        if (!result) return online_users;

        MYSQL_ROW row;
        while ((row= mysql_fetch_row(result))!= nullptr){
            int user_id = std::stoi(row[0]);
            UserInfo user_info;
            if (getUserInfo(user_id,user_info)){
                online_users[user_id] = user_info;
            }
        }
        mysql_free_result(result);
        return online_users;
    }

    int UserManager::getSocketFdByUserId(int user_id) {
        std::lock_guard<std::mutex> lock(online_user_mutex_);
        auto it = online_users_.find(user_id);
        if (it!=online_users_.end()) return it->second;
        return -1;
    }
}