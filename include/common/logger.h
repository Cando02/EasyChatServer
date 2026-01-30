//
// Created by Cando on 2026/1/30.
//

#ifndef EASYCHATSERVER_LOGGER_H
#define EASYCHATSERVER_LOGGER_H


#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <ctime>
#include <iomanip>


namespace easychat{
    // 日志级别枚举
    enum class LogLevel{
        LOG_DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3
    };
    // 日志类-单例模式
    class Logger{
    public:
        static Logger& getInstance();
        // 初始化日志系统
        bool init(const std::string& file_path,
                  LogLevel level,bool console_output= true,
                  size_t max_size=100*1024*1024,int backup_count=5);
        // 记录日志
        void log(LogLevel level,const std::string& message);
        //便捷日志方法
        void debug(const std::string message);
        void info(const std::string message);
        void warn(const std::string message);
        void error(const std::string message);
        // 刷新日志缓冲区
        void flush();
        // 关闭日志系统
        void close();
    private:
        Logger()=default;
        ~Logger();
        // 禁止拷贝与赋值
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        //检查日志文件大小,大了轮转
        void checkRotation();
        // 获取日志级别字符串
        std::string getLevelString(LogLevel level);
        // 获取当前时间字符串
        std::string getCurrentTime();
        // 格式化日志消息
        std::string formatMessage(LogLevel level,const std::string &message);

        std::ofstream log_file_;// 日志工作流
        std::mutex log_mutex_;  //日志互斥锁
        LogLevel min_level_;    //最小日志级别
        bool console_output_;   // 是否输出到控制台
        std::string file_path_; //日志文件路径
        size_t max_size_;   //日志文件最大大小
        int backup_count_;  //日志文件备份数量
        bool initialized_;  // 是否已经初始化
    };
    // 日志流类-支持流式日志
    class LogStream{
    public:
        LogStream(LogLevel level);
        ~LogStream();
        // 支持流式输出
        template<typename T> LogStream& operator<<(const T& value){
            buffer_ << value;
            return *this;
        }
    private:
        LogLevel level_;
        std::ostringstream buffer_;
    };

    //便捷宏定义
    #define LOG_DEBUG() LogStream(LogLevel::LOG_DEBUG);
    #define LOG_INFO() LogStream(LogLevel::INFO)
    #define LOG_WARN() LogStream(LogLevel::WARN)
    #define LOG_ERROR() LogStream(LogLevel::ERROR)
}

#endif //EASYCHATSERVER_LOGGER_H
