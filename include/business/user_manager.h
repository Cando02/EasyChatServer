//
// Created by Cando on 2026/1/30.
//

#ifndef EASYCHATSERVER_USER_MANAGER_H
#define EASYCHATSERVER_USER_MANAGER_H

#include "database/connection_pool.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace easychat{
    // 用户信息结构体
    struct UserInfo{
        int id;
        std::string username;
        std::string password;
        std::string nickname;
        std::string avatar; //头像
        int status; // 0-离线，1-在线
    };
    // 用户管理类
    class UserManager{
    public:
        static UserManager& getInstance();
        //初始化
        void init();
        //用户注册
        bool registerUser(const std::string& username,
                          const std::string& password,
                          const std::string& nickname="");
        // 用户登录
        bool loginUser(const std::string& username,
                       const std::string& password,
                       int &user_id);
        // 获取用户信息
        bool getUserInfo(int user_id,UserInfo& user_info);
        bool getUserInfo(const std::string& username,UserInfo& user_info);
        //更新用户状态
        bool updateUserStatus(int user_id,int status);
        // 用户上线
        bool userOnline(int user_id,int socket_fd,const std::string& ip,int port);
        // 用户下线
        bool userOffline(int user_id);
        // 检查用户是否在线
        bool isUserOnline(int user_id);
        //获取在线用户列表
        std::unordered_map<int,UserInfo>getOnlineUsers();
        // 根据用户ID获取SocketFd
        int getSocketFdByUserId(int user_id);
    private:
        UserManager();
        ~UserManager();
        // 禁止拷贝和赋值
        UserManager(const UserManager&) = delete;
        UserManager& operator=(const UserManager&) = delete;
        // 密码加密
        std::string encryptPassword(const std::string & password);
        //在线用户映射（用户ID->SocketFd）
        std::unordered_map<int,int> online_users_;
        std::mutex online_user_mutex_;
        // 连接池引用
        ConnectionPool& conn_pool_;
    };
}

#endif //EASYCHATSERVER_USER_MANAGER_H
