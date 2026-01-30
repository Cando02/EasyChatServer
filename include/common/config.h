//
// Created by Cando on 2026/1/30.
//

#ifndef EASYCHATSERVER_CONFIG_H
#define EASYCHATSERVER_CONFIG_H

#include<string>
#include <map>
#include <fstream>
#include <sstream>
namespace easychat{
    //配置类 - 单例模式
    class Config{
    public:
        static Config& getInstance();
        // 加载配置文件
        bool load(const std::string & config_file);
        // 获取字符串配置
        std::string getString(const std::string& key,const std::string& default_value="");
        // 获取整数配置
        int getInt(const std::string & key,int default_value=0);
        // 获取布尔配置
        bool getBool(const std::string& key,bool default_value= false);
        // 获取端口配置
        uint16_t getPort(const std::string &key,uint16_t default_value = 0);
    private:
        Config() = default;
        ~Config() = default;

        // 禁止拷贝和赋值
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        // 解析配置行
        void parseLine(const std::string& line,std::string& section);
        // 去除字符串首尾空白
        std::string trim(const std::string& str);
        // 配置存储：section->(key->value)
        std::map<std::string,std::map<std::string,std::string>> config_data_;
        std::string current_section_;
    };
}

#endif //EASYCHATSERVER_CONFIG_H
