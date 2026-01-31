//
// Created by Cando on 2026/1/31.
//
#include "../../include/common/daemon.h"
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <fstream>

namespace easychat{
    bool Daemon::start() {
        // 创建子进程，父进程退出
        // fork() 创建子进程，返回两次：父进程返回子进程 PID，子进程返回 0
        pid_t pid = fork();
        if (pid<0){
            std::cerr<<"Failed to fork process: "<<strerror(errno)<<std::endl;
            return false;
        }
        if (pid>0){
            // 父进程退出，子进程继续运行
            exit(EXIT_SUCCESS);
        }
        // 创建新会话
        if (setsid()<0){
            std::cerr<<"Failed to create new session: "<<strerror(errno)<<std::endl;
            return false;
        }
        // 再次创建fork,确保进程不会重新获取控制终端
        pid = fork();
        if (pid<0){
            std::cerr<<"Failed to fork process (second time): "<<strerror(errno)<<std::endl;
            return false;
        }
        if (pid>0){
            // 第一个子进程退出，第二个子进程继续运行
            exit(EXIT_SUCCESS);
        }
        // 修改工作目录到根目录
        if (chdir("/")<0){
            std::cerr<<"Failed to change directory: 0"<<strerror(errno)<<std::endl;
            return false;
        }
        // 重设文件权限掩码
        umask(0);
        // 关闭所有文件描述符
        for (int fd=0;fd<getdtablesize();++fd){
            close(fd);
        }
        // 重定向标准输入、输出、错误到 /dev/null
        int fd0 = open("/dev/null",O_RDWR);
        int fd1 = dup(fd0);
        int fd2 = dup(fd0);

        if (fd0!=STDIN_FILENO || fd1!=STDOUT_FILENO || fd2!=STDERR_FILENO){
            std::cerr<<"Unexpected file descriptors"<<std::endl;
            return false;
        }
        // 忽略SIGCHLD信号
        signal(SIGCHLD,SIG_IGN);
        // 忽略SIGHUP信号
        signal(SIGHUP,SIG_IGN);
        std::cout<<"Daemon started successfully (PID: "<<getpid()<<")"<<std::endl;
        return true;
    }

    bool Daemon::stop(const std::string &pid_file) {
        // 读取PID
        int pid = getPid(pid_file);
        if (pid==-1){
            std::cerr<<"Failed to read PID file"<<std::endl;
            return false;
        }
        // 发送SIGTERM信号
        if (kill(pid,SIGTERM)<0){
            std::cerr<<"Failed to send SIGTERM to process"<<pid<<": "<<strerror(errno)<<std::endl;
            return false;
        }
        // 等待进程退出
        int timeout = 10;
        int count = 0;
        while (kill(pid,0)==0 && count<timeout){
            sleep(1);
            count++;
        }
        //检查进程是否退出
        if (kill(pid,0)==0){
            // 进程仍在运行，强制终止
            std::cerr<<"Process did not stop gracefully, forcing..."<<std::endl;
            if (kill(pid,SIGKILL)<0){
                std::cerr<<"Failed to send SIGKILL to process "<<pid<<std::endl;
                return false;
            }
            sleep(1);
        }
        // 删除PID文件
        removePidFile(pid_file);
        std::cout<<"Daemon stopped successfully"<<std::endl;
        return true;
    }

    bool Daemon::isRunning(const std::string &pid_file) {
        int pid = getPid(pid_file);
        if (pid==-1) return false;
        // 检查进程是否存在,判断能否想进程pid发送信息
        return kill(pid,0)==0;
    }

    int Daemon::getPid(const std::string &pid_file) {
        std::ifstream file(pid_file);
        if (!file.is_open()){
            return -1;
        }
        int pid;
        file>>pid;
        file.close();
        return pid;
    }

    bool Daemon::writePidFile(const std::string &pid_file) {
        std::ofstream file(pid_file);
        if (!file.is_open()){
            std::cerr<<"Failed to open PID file: "<<pid_file<<std::endl;
            return false;
        }
        //写入当前进程PID
        file <<getpid();
        file.close();
        return true;
    }

    bool Daemon::removePidFile(const std::string &pid_file) {
        return std::remove(pid_file.c_str())==0;
    }
}