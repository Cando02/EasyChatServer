//
// Created by Cando on 2026/1/31.
//

#ifndef EASYCHATSERVER_DAEMON_H
#define EASYCHATSERVER_DAEMON_H

#include <string>

namespace easychat{
    // 守护进程类
    class Daemon{
    public:
        // 启动守护进程
        static bool start();
        // 停止守护进程
        static bool stop(const std::string &pid_file);
        // 检查守护进程状态
        static bool isRunning(const std::string &pid_file);
        // 获取守护进程PID
        static int getPid(const std::string &pid_file);
        // 写入PID文件
        static bool writePidFile(const std::string &pid_file);
        // 删除PID文件
        static bool removePidFile(const std::string &pid_file);
    private:
        Daemon()=delete;
        ~Daemon()=delete;
    };
}

#endif //EASYCHATSERVER_DAEMON_H
