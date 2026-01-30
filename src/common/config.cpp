//
// Created by Cando on 2026/1/30.
//
#include "../../include/common/config.h"
#include <iostream>
#include <algorithm>

namespace easychat{
    Config& Config::getInstance() {
        static Config instance;
        return instance;
    }

    bool Config::load(const std::string &config_file) {
        // 打开配置文件
        std::ifstream file(config_file);
        if (!file.is_open()){
            std::cerr<<"Failed to open config file: "<<config_file<<std::endl;
            return false;
        }
        std::string line;
        int line_number = 0;
        // 逐行读取配置文件
        while (std::getline(file,line)){
            line_number++;
            // 去除首尾空白
            line = trim(line);
            // 跳过空行和注释
            if (line.empty() || line[0]=='#' || line[0]==';') continue;
            // 解析配置行
            parseLine(line,current_section_);
        }
        file.close();
        std::cout<<"Config loaded from: "<<config_file<<std::endl;
        return true;
    }

    void Config::parseLine(const std::string &line, std::string &section) {
        //检查是否为section
        if (line[0]=='[' && line.back()==']'){
            // 提取section名称
            section = line.substr(1,line.length()-2);
            section = trim(section);
            return;
        }
        // 解析键值对
        size_t equal_pos = line.find('=');
        if (equal_pos==std::string::npos){
            std::cerr<<"Invalid config line: "<<line<<std::endl;
            return;
        }
        // 提取键值对
        std::string key = trim(line.substr(0,equal_pos));
        std::string value = trim(line.substr(equal_pos+1));

        // 去除两端引号
        if (value.length()>=2 && ((value[0]=='"'&&value.back()=='"')||(value[0]=='\''&&value.back()=='\''))){
            value = value.substr(1,value.length()-2);
        }
        // 存储配置
        config_data_[section][key] = value;
    }

    std::string Config::getString(const std::string &key, const std::string &default_value) {
        // 解析key
        size_t dot_pos = key.find('.');
        if (dot_pos==std::string::npos){
            return default_value;
        }
        std::string section = key.substr(0,dot_pos);
        std::string config_key = key.substr(dot_pos+1);
        //查找配置
        auto section_it = config_data_.find(section);
        if (section_it==config_data_.end()){
            return default_value;
        }

        auto key_it = section_it->second.find(config_key);
        if (key_it==section_it->second.end()) return default_value;

        return key_it->second;
    }

    int Config::getInt(const std::string &key, int default_value) {
        std::string value = getString(key);
        if (value.empty()) return default_value;
        try {
            return std::stoi(value);
        }catch (const std::exception& e){
            std::cerr<<"Invalid integer config: "<<key<<" = "<<value<<std::endl;
            return default_value;
        }
    }

    bool Config::getBool(const std::string &key, bool default_value) {
        std::string value = getString(key);
        if (value.empty()){
            return default_value;
        }
        //转换为小写
        std::transform(value.begin(),value.end(),value.begin(),::tolower);
        //解析布尔值
        if (value=="true" || value=="yes"||value == "1"||value=="on"){
            return true;
        }else if (value=="false" || value=="no" || value=="0" || value=="off"){
            return false;
        }
        return default_value;
    }

    uint16_t Config::getPort(const std::string &key, uint16_t default_value) {
        int port = getInt(key,default_value);
        if (port<0 || port>65535){
            std::cerr<<"Invalid port number: "<<port<<std::endl;
            return default_value;
        }
        return static_cast<uint16_t>(port);
    }

    std::string Config::trim(const std::string &str) {
        //去除字符串首尾空白字符
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start==std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start,end-start+1);
    }
}