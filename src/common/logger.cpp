//
// Created by Cando on 2026/1/31.
//
#include "../../include/common/logger.h"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <thread>

namespace easychat{
    Logger::~Logger() {
        close();
    }

    Logger &Logger::getInstance() {
        static Logger instance;
        return instance;
    }

    bool Logger::init(const std::string &file_path, easychat::LogLevel level, bool console_output, size_t max_size,
                      int backup_count) {
        std::lock_guard<std::mutex> lock(log_mutex_);
        if (initialized_) return true;

        file_path_ = file_path;
        min_level_ = level;
        console_output_ = console_output;
        max_size_ = max_size;
        backup_count_ = backup_count;
        // 创建日志目录
        std::filesystem::path log_path(file_path);
        std::filesystem::path log_dir = log_path.parent_path();
        if (!log_dir.empty() && !std::filesystem::exists(log_dir)){
            std::filesystem::create_directories(log_dir);
        }
        // 打开日志文件
        log_file_.open(file_path,std::ios::out | std::ios::app);
        if (!log_file_.is_open()){
            std::cerr<<"Failed to open log file: "<<file_path<<std::endl;
            return false;
        }

        initialized_ = true;
        // 注意：不能在这里调用 info() 方法，因为已经持有 log_mutex_ 锁
        // 会导致死锁，因为 info() 会调用 log()，而 log() 也会尝试获取锁
        // 所以直接使用 std::cout 输出初始化信息
        std::cout<<"Logger initialized, level: "<<getLevelString(level)<<", file: "<<file_path<<std::endl;
        return true;
    }

    void Logger::log(easychat::LogLevel level, const std::string &message) {
        if (level<min_level_) return;
        std::lock_guard<std::mutex> lock(log_mutex_);
        if (!initialized_) return;
        // 格式化日志消息
        std::string formatted_msg = formatMessage(level,message);
        //写入日志文件
        log_file_<<formatted_msg<<std::endl;
        log_file_.flush();
        // 输出到控制台
        if (console_output_){
            std::cout<<formatted_msg<<std::endl;
        }
        //检查日志轮转
        checkRotation();
    }

    void Logger::debug(const std::string& message) {
        log(LogLevel::LOG_DEBUG,message);
    }

    void Logger::info(const std::string &message) {
        log(LogLevel::INFO,message);
    }

    void Logger::warn(const std::string &message) {
        log(LogLevel::WARN,message);
    }

    void Logger::error(const std::string &message) {
        log(LogLevel::ERROR,message);
    }

    void Logger::flush() {
        std::lock_guard<std::mutex> lock(log_mutex_);
        if (log_file_.is_open()){
            log_file_.flush();
        }
    }

    void Logger::close() {
        std::lock_guard<std::mutex> lock(log_mutex_);
        if (log_file_.is_open()){
            // 注意：不能在这里调用 info() 方法，因为已经持有 log_mutex_ 锁
            // 会导致死锁，因为 info() 会调用 log()，而 log() 也会尝试获取锁
            // 所以直接使用 std::cout 输出关闭信息
            std::cout<<"Logger closed"<<std::endl;
            log_file_.close();
        }
        initialized_ = false;
    }

    void Logger::checkRotation() {
        if (!log_file_.is_open()) return;
        // 获取当前文件大小
        std::streampos file_size = log_file_.tellp();
        if (file_size >= static_cast<std::streampos>(max_size_)){
            //关闭当前日志
            log_file_.close();
            // 删除最旧的备份文件
            std::string oldest_back = file_path_+"."+std::to_string(backup_count_);
            std::filesystem::remove(oldest_back);
            // 重命名备份文件
            for (int i=backup_count_-1;i>=1;--i){
                std::string old_backup = file_path_+"."+std::to_string(i);
                std::string new_backup = file_path_+"."+std::to_string(i+1);
                std::filesystem::rename(old_backup,new_backup);
            }
            // 重命名当前日志文件为第一个备份
            std::string first_back = file_path_+".1";
            std::filesystem::rename(file_path_,first_back);
            // 重新打开日志文件
            log_file_.open(file_path_,std::ios::out | std::ios::app);
            if (!log_file_.is_open()){
                std::cerr<<"Failed to reopen log file after rotation"<<std::endl;
            }
            // 注意：不能在这里调用 info() 方法，因为已经持有 log_mutex_ 锁
            // 会导致死锁，因为 info() 会调用 log()，而 log() 也会尝试获取锁
            // 所以直接使用 std::cout 输出轮转信息
            std::cout<<"Log file rotated"<<std::endl;
        }
    }

    std::string Logger::getLevelString(easychat::LogLevel level) {
        switch (level) {
            case LogLevel::LOG_DEBUG: return "LOG_DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default:return "UNKNOWN";
        }
    }

    std::string Logger::getCurrentTime() {
        //获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch())%1000;
        // 格式化时间字符串
        std::ostringstream oss;
        oss<<std::put_time(std::localtime(&time_t_now),"%Y-%m-%d %H:%M:%S");
        oss<<'.'<<std::setfill('0')<<std::setw(3)<<ms.count();
        return oss.str();
    }

    std::string Logger::formatMessage(easychat::LogLevel level, const std::string &message) {
        std::ostringstream oss;
        oss<<"["<<getCurrentTime()<<"]"
        <<"["<<getLevelString(level)<<"]"
        <<"["<<std::this_thread::get_id()<<"]"
        <<message;
        return oss.str();
    }

    LogStream::LogStream(easychat::LogLevel level) :level_(level){}
    LogStream::~LogStream() {
        std::string message = buffer_.str();
        Logger::getInstance().log(level_,message);
    }
}