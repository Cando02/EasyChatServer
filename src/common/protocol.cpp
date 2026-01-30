//
// Created by 30501 on 2026/1/29.
//
#include "../../include/common/protocol.h"
#include<cstring>
#include<arpa/inet.h>

namespace easychat{
    // 无参初始化
    Message::Message()
    : type_(MessageType::MSG_TYPE_ERROR),
    user_id_(0),data_(""){}
    // 有参初始化
    Message::Message(MessageType type)
    : type_(type),user_id_(0),data_(""){}
    // 全参初始化
    Message::Message(MessageType type,uint32_t user_id,
    const std::string& data)
    :type_(type),user_id_(user_id),data_(data){}

    // 序列化实现
    std::vector<char> Message::serialize() const {
        // 计算消息总长度：头部(12字节) + 消息体长度
        size_t total_length = sizeof(MessageHeader)+data_.size();
        // 建立缓冲区
        std::vector<char> buffer(total_length);

        // 构建消息头部
        MessageHeader header;
        header.length = hostToNetwork32(
        static_cast<uint32_t>(total_length));
        header.type = hostToNetwork32(
            static_cast<uint32_t>(type_));
        header.user_id = hostToNetwork32(user_id_);

        // 将头部拷贝到缓冲区
        std::memcpy(buffer.data(),&header,sizeof(MessageHeader));

        // 将消息体拷贝到缓冲区
        if(!data_.empty()){
            std::memcpy(buffer.data()+sizeof(MessageHeader),
            data_.data(),data_.size());
        }

        return buffer;
    }
    // 反序列化实现
    Message Message::deserialize(const char *data, size_t length) {
        // 检查数据长度
        if (length<sizeof(MessageHeader)){
            return Message(MessageType::MSG_TYPE_ERROR);
        }
        // 解析消息头部
        MessageHeader header;
        std::memcpy(&header,data,sizeof(MessageHeader));

        // 转换字节序
        uint32_t total_length = networkTOHost32(header.length);
        uint32_t type = networkTOHost32(header.type);
        uint32_t user_id = networkTOHost32(header.user_id);

        // 检查消息长度
        if (total_length!=length){
            return Message(MessageType::MSG_TYPE_ERROR);
        }

        // 提取消息体
        std::string message_data;
        if (length>sizeof(MessageHeader)){
            message_data.assign(data+sizeof (MessageHeader),
                            length-sizeof (MessageHeader));
        }

        return Message(static_cast<MessageType>(type),user_id,message_data);
    }
    size_t Message::getTotalLength() const {
        return sizeof (MessageHeader)+data_.size();
    }
}