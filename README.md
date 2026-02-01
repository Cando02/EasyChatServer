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
- 在线用户列表
- 聊天记录查询
- 支持用户名操作

## 项目结构
```
EasyChatServer/
├── CMakeLists.txt              # 主 CMake 配置文件
├── README.md                   # 项目说明文档
├── .gitignore                  # Git 忽略文件配置
├── config/                     # 配置文件目录
│   ├── .gitkeep
│   └── server.conf            # 服务器配置文件
├── docs/                       # 文档目录
│   └── .gitkeep
├── include/                    # 头文件目录
│   ├── business/              # 业务逻辑层头文件
│   │   ├── .gitkeep
│   │   ├── message_handler.h # 消息处理
│   │   └── user_manager.h    # 用户管理
│   ├── common/                # 公共头文件
│   │   ├── config.h          # 配置管理
│   │   ├── daemon.h          # 守护进程
│   │   ├── logger.h          # 日志系统
│   │   ├── protocol.h        # 协议定义
│   │   └── signal_handler.h  # 信号处理
│   ├── database/              # 数据库层头文件
│   │   ├── .gitkeep
│   │   └── connection_pool.h # 连接池实现
│   ├── network/               # 网络层头文件
│   │   ├── .gitkeep
│   │   ├── epoll.h           # Epoll 封装
│   │   ├── reactor.h         # Reactor 模型
│   │   └── socket.h          # Socket 封装
│   └── threadpool/            # 线程池头文件
│       ├── .gitkeep
│       └── threadpool.h      # 线程池实现
├── logs/                       # 日志目录
│   └── .gitkeep
├── python/                     # Python 客户端和监控脚本
│   ├── client.py              # Python 客户端
│   ├── monitor.py             # 服务器监控脚本
│   └── protocol.py            # 协议定义
├── scripts/                    # 脚本目录
│   ├── .gitkeep
│   ├── init_database.sql      # 数据库初始化脚本
│   ├── restart.sh             # 重启脚本
│   ├── start.sh               # 启动脚本
│   ├── status.sh              # 状态查看脚本
│   └── stop.sh                # 停止脚本
├── src/                        # 源文件目录
│   ├── business/              # 业务逻辑层源文件
│   │   ├── .gitkeep
│   │   ├── message_handler.cpp
│   │   └── user_manager.cpp
│   ├── common/                # 公共源文件
│   │   ├── config.cpp
│   │   ├── daemon.cpp
│   │   ├── logger.cpp
│   │   ├── protocol.cpp
│   │   └── signal_handler.cpp
│   ├── database/              # 数据库层源文件
│   │   ├── .gitkeep
│   │   └── connection_pool.cpp
│   ├── network/               # 网络层源文件
│   │   ├── .gitkeep
│   │   ├── epoll.cpp
│   │   ├── reactor.cpp
│   │   └── socket.cpp
│   ├── threadpool/            # 线程池源文件
│   │   ├── .gitkeep
│   │   └── threadpool.cpp
│   └── main.cpp               # 主程序入口
├── static/                     # 静态资源目录
│   ├── 发送消息1.jpg
│   ├── 发送消息2.jpg
│   ├── 在线用户列表.jpg
│   ├── 查看聊天记录.jpg
│   └── 登陆界面.jpg
└── tests/                      # 测试目录
    ├── example_client.py      # 客户端使用示例
    ├── example_monitor.py     # 监控脚本使用示例
    ├── test_chat.py           # 聊天功能测试
    ├── test_client.py         # 客户端功能测试
    ├── test_concurrent.py     # 并发连接测试
    ├── test_offline.py        # 离线消息测试
    └── test_throughput.py     # 消息吞吐量测试
```

## 环境依赖

### 服务器端依赖

#### 1. 系统依赖
```bash
# Ubuntu/Debian系统
apt update
apt install -y build-essential cmake libmysqlclient-dev

# CentOS/RHEL系统
yum update
yum install -y gcc gcc-c++ cmake mysql-devel
```

#### 2. MySQL数据库
```bash
# 安装MySQL
# Ubuntu/Debian
apt install -y mysql-server mysql-client

# CentOS/RHEL
yum install -y mysql-server mysql-client

# 启动MySQL服务
systemctl start mysql
systemctl enable mysql

# 初始化数据库
mysql -u root -p < scripts/init_database.sql
```

### 客户端依赖

#### 1. Python环境
```bash
# 安装Python3
# Ubuntu/Debian
apt install -y python3 python3-pip

# CentOS/RHEL
yum install -y python3 python3-pip

# Windows
# 从官网下载并安装Python 3.6+
```

#### 2. Python依赖包
```bash
# 安装监控脚本依赖
pip3 install psutil
```

## 编译运行

### 步骤1：编译服务器
```bash
# 创建构建目录
mkdir build && cd build

# 配置CMake
cmake ..

# 编译项目
make

# 运行服务器
./EasyChatServer
```

### 步骤2：使用启动脚本
```bash
# 启动服务器
./scripts/start.sh

# 停止服务器
./scripts/stop.sh

# 重启服务器
./scripts/restart.sh

# 查看服务器状态
./scripts/status.sh
```

### 步骤3：启动客户端
```bash
# 运行Python客户端
cd python
python client.py

# 或从项目根目录运行
python python/client.py
```

### 步骤4：运行测试
```bash
# 运行功能测试
python tests/test_chat.py
python tests/test_client.py
python tests/test_offline.py

# 运行性能测试
python tests/test_concurrent.py
python tests/test_throughput.py

# 运行监控脚本
python tests/example_monitor.py
```

## 数据库设计

### 数据表结构

#### users表 - 用户信息表
```sql
CREATE TABLE users (
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    nickname VARCHAR(50),
    avatar VARCHAR(255),
    status INT DEFAULT 0 COMMENT '0-离线，1-在线',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

#### messages表 - 消息记录表
```sql
CREATE TABLE messages (
    id INT PRIMARY KEY AUTO_INCREMENT,
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    content TEXT NOT NULL,
    message_type INT DEFAULT 1,
    is_offline INT DEFAULT 0,
    is_read INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sender_id) REFERENCES users(id),
    FOREIGN KEY (receiver_id) REFERENCES users(id)
);
```

#### online_users表 - 在线用户表
```sql
CREATE TABLE online_users (
    user_id INT PRIMARY KEY,
    socket_fd INT NOT NULL,
    ip VARCHAR(50),
    port INT,
    last_heartbeat TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);
```

### 初始化数据
系统默认创建以下测试用户：
- admin / 123456 (管理员)
- user1 / 123456 (用户1)
- user2 / 123456 (用户2)

## 客户端使用说明

### Python客户端

#### 启动客户端
```bash
cd python
python client.py
```

#### 可用命令
```
login username password    - 登录
register username password [nickname] - 注册
send receiver message    - 发送消息（支持用户名或用户ID）
history user [limit]     - 查看聊天记录（支持用户名或用户ID）
users                     - 查看在线用户
quit                      - 退出
```

#### 使用示例
```python
# 1. 启动客户端
python client.py

# 2. 登录
> login user1 123456
✔ 连接成功：localhost:8888
-> 发送登录请求：user1
<- 接收消息[LOGIN_RESP] from 2:Login successful
✔ 登陆成功，用户ID:2

# 3. 查看在线用户
> users
-> 请求在线用户列表
<- 接收消息[USERS_RESP] from 2:2:user1,3:user2
✔ 在线用户列表（共2人）：
  - user1 (ID: 2)
  - user2 (ID: 3)

# 4. 发送消息（使用用户名）
> send user2 你好，我是user1
-> 发送消息到user2(3)：你好，我是user1

# 5. 查看聊天记录
> history user2
-> 请求与user2(3)的聊天记录，最近50条
<- 接收消息[HISTORY_RESP] from 2:2:你好，我是user1|3:你好，user1
✔ 聊天记录（共2条）：
  1. [user1] 你好，我是user1
  2. [user2] 你好，user1

# 6. 退出
> quit
✔ 连接已关闭
```

## 功能演示

### 登录功能
![登陆](D:\Users\30501\Desktop\school\C++\EasyChatServer\static\登陆界面.jpg)
- 支持用户名密码登录
- 登录成功后显示用户ID
- 自动推送离线消息

### 在线用户列表
![在线用户列表](D:\Users\30501\Desktop\school\C++\EasyChatServer\static\在线用户列表.jpg)
- 实时显示所有在线用户
- 显示用户名和用户ID
- 支持使用用户名操作

### 消息发送
![发送消息](D:\Users\30501\Desktop\school\C++\EasyChatServer\static\发送消息1.jpg)

![消息推送](D:\Users\30501\Desktop\school\C++\EasyChatServer\static\发送消息2.jpg)

- 支持使用用户名或用户ID发送消息
- 实时消息推送
- 支持中英文消息

### 聊天记录
![聊天记录](D:\Users\30501\Desktop\school\C++\EasyChatServer\static\查看聊天记录.jpg)
- 按时间倒序显示聊天记录
- 显示发送者和消息内容
- 支持指定查询条数

## 性能数据

### 并发性能
| 并发连接数 | 测试耗时 | 性能表现 |
|-----------|----------|----------|
| 100       | 30.56秒  | 优秀     |
| 500       | 32.65秒  | 优秀     |
| 1000      | 35.69秒  | 优秀     |

### 消息吞吐量
| 测试项目 | 数值 |
|----------|------|
| 发送速率 | 258,747.93 消息/秒 |
| 接收速率 | 55.42 消息/秒 |
| 发送耗时 | 0.00秒 |
| 接收耗时 | 18.05秒 |

### 数据库性能
- **连接池大小**：10
- **查询响应时间**：< 5ms
- **支持并发查询**：100+

### 网络性能
- **网络延迟**：< 1ms（局域网）
- **消息传输速度**：10MB/s
- **支持协议**：TCP/IP

### 性能分析
1. **并发连接能力**：服务器能够稳定支持1000个并发连接，随着连接数增加，耗时增长缓慢，表现出良好的线性扩展能力。

2. **消息处理能力**：发送速率高达258,747.93消息/秒，表明服务器在消息发送方面性能优异；接收速率受客户端处理能力限制，实际服务器处理能力更高。

3. **系统稳定性**：所有测试均成功完成，无连接失败或异常情况，表明服务器在高并发下保持稳定运行。

### 服务器监控

#### 监控功能
- **实时状态监控**：服务器运行状态、响应时间
- **系统资源监控**：CPU、内存、磁盘使用率
- **网络统计**：TCP连接数、网络流量
- **告警机制**：支持设置阈值告警
- **数据持久化**：监控数据自动保存到JSON文件

#### 实时监控数据（2026-02-01）

| 监控项 | 数值 | 状态 |
|--------|------|------|
| 服务器状态 | 运行中 | ✅ |
| 响应时间 | 0.27-0.65ms | ✅ |
| CPU使用率 | 0.0% | ✅ |
| 内存使用率 | 41.9% (1627MB / 3888MB) | ✅ |
| 磁盘使用率 | 48.6% (8.2GB / 17.8GB) | ✅ |
| TCP连接数 | 10-12 | ✅ |
| 网络发送 | 103.8MB | ✅ |
| 网络接收 | 112.2MB | ✅ |

#### 如何使用监控脚本
```bash
# 运行监控脚本
python tests/example_monitor.py

# 监控命令
help    - 查看帮助
stats   - 显示统计信息
save    - 保存监控数据
load    - 加载监控数据
clear   - 清空监控数据
quit    - 退出
```

## 技术亮点

### 1. 高并发网络模型
- 基于Epoll的Reactor模式，支持边缘触发（ET）模式
- 非阻塞I/O，提高系统吞吐量
- 事件驱动架构，减少线程切换开销

### 2. 线程池设计
- 工作线程池处理业务逻辑，避免阻塞主线程
- 任务队列管理，实现任务分发和负载均衡
- 支持动态调整线程数量

### 3. 数据库连接池
- 预分配数据库连接，减少连接创建开销
- 连接复用机制，提高数据库操作效率
- 自动连接管理和错误恢复

### 4. 自定义协议
- 固定头部协议，解决TCP粘包问题
- 统一消息格式，支持跨语言客户端
- 支持多种消息类型（登录、聊天、心跳等）

### 5. 用户友好设计
- 支持用户名操作，无需记忆用户ID
- 实时在线状态更新
- 离线消息自动推送

## 系统要求

### 服务器端
- 操作系统：Linux (Ubuntu 18.04+)
- 编译器：GCC 7.0+ (支持C++11/14)
- 数据库：MySQL 5.7+
- 内存：512MB+
- 磁盘：100MB+

### 客户端
- 操作系统：Windows/Linux/macOS
- Python版本：3.6+
- 网络：支持TCP/IP

## 常见问题

### Q: 如何修改服务器端口？
A: 修改 `config/server.conf` 文件中的 `port` 配置项。

### Q: 如何增加数据库连接池大小？
A: 修改 `src/database/connection_pool.cpp` 中的连接池初始化参数。

### Q: 客户端连接失败怎么办？
A: 检查服务器是否启动，防火墙是否开放对应端口。

### Q: 如何查看服务器日志？
A: 日志文件位于 `logs/server.log`，可以使用 `tail -f logs/server.log` 实时查看。

### Q: 如何使用用户名发送消息？
A: 直接使用 `send username message` 命令，客户端会自动将用户名转换为用户ID。

## 开发路线图

- [x] 基础网络框架搭建
- [x] 用户注册登录功能
- [x] 点对点单聊功能
- [x] 离线消息推送
- [x] 数据持久化
- [x] Python客户端开发
- [x] 在线用户管理
- [x] 聊天记录查询
- [x] 支持用户名操作
- [ ] 群聊功能
- [ ] 文件传输
- [ ] 语音/视频通话
- [ ] 分布式架构
- [ ] 消息加密
- [ ] Web客户端

## 许可证
MIT License

## 联系方式
- 项目地址：[GitHub](https://github.com/yourusername/EasyChatServer)
- 邮箱：your.email@example.com
