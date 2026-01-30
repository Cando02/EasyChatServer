# EasyChatServer

轻量级高并发分布式聊天推送服务器

## 项目简介
基于 epoll + 线程池的高性能即时通讯服务器，支持千级并发连接。





## 技术栈
- C++11/14
- Linux Epoll
- MySQL
- CMake
- Python（客户端）

## 功能特性
- 高并发连接管理
- 用户注册登录
- 点对点单聊
- 离线消息推送
- 数据持久化

## 项目结构
```
EasyChatServer/
├── CMakeLists.txt              # 主 CMake 配置文件
├── README.md                   # 项目说明文档
├── .gitignore                  # Git 忽略文件配置
├── config/                     # 配置文件目录
│   └── server.conf            # 服务器配置文件
├── include/                    # 头文件目录
│   ├── common/                # 公共头文件
│   │   ├── protocol.h        # 协议定义
│   │   ├── logger.h          # 日志系统
│   │   └── utils.h           # 工具函数
│   ├── network/               # 网络层头文件
│   │   ├── epoll.h           # Epoll 封装
│   │   ├── socket.h          # Socket 封装
│   │   └── reactor.h         # Reactor 模型
│   ├── threadpool/            # 线程池头文件
│   │   └── threadpool.h      # 线程池实现
│   ├── database/              # 数据库层头文件
│   │   └── connection_pool.h # 连接池实现
│   └── business/             # 业务逻辑层头文件
│       ├── user_manager.h    # 用户管理
│       ├── message_handler.h # 消息处理
│       └── offline_message.h  # 离线消息
├── src/                       # 源文件目录
│   ├── common/               # 公共源文件
│   │   ├── protocol.cpp
│   │   ├── logger.cpp
│   │   └── utils.cpp
│   ├── network/              # 网络层源文件
│   │   ├── epoll.cpp
│   │   ├── socket.cpp
│   │   └── reactor.cpp
│   ├── threadpool/           # 线程池源文件
│   │   └── threadpool.cpp
│   ├── database/             # 数据库层源文件
│   │   └── connection_pool.cpp
│   ├── business/            # 业务逻辑层源文件
│   │   ├── user_manager.cpp
│   │   ├── message_handler.cpp
│   │   └── offline_message.cpp
│   └── main.cpp             # 主程序入口
├── scripts/                  # 脚本目录
│   ├── start.sh             # 启动脚本
│   ├── stop.sh              # 停止脚本
│   └── restart.sh           # 重启脚本
├── logs/                     # 日志目录（运行时生成）
├── tests/                    # 测试目录
│   └── test_main.cpp        # 测试入口
├── python/                   # Python 客户端和监控脚本
│   ├── client.py            # Python 客户端
│   └── monitor.py           # 监控脚本
└── docs/                     # 文档目录
    ├── design.md            # 设计文档
    └── api.md               # API 文档
```

## 编译运行
```bash
mkdir build && cd build
cmake ..
make
./EasyChatServer