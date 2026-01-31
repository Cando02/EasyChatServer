#!/bin/bash

# EasyChatServer 启动脚本

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# 配置变量
SERVER_NAME="EasyChatServer"
SERVER_BIN="$PROJECT_DIR/build/bin/$SERVER_NAME"
PID_FILE="$PROJECT_DIR/$SERVER_NAME.pid"
LOG_DIR="$PROJECT_DIR/logs"

# 创建日志目录
mkdir -p "$LOG_DIR"

# 检查服务器可执行文件是否存在
if [ ! -f "$SERVER_BIN" ]; then
    echo "Error: $SERVER_BIN not found"
    echo "Please build the project first"
    exit 1
fi

# 检查服务器是否已经运行
if [ -f "$PID_FILE" ]; then
    PID=$(cat "$PID_FILE")
    if ps -p "$PID" > /dev/null 2>&1; then
        echo "$SERVER_NAME is already running (PID: $PID)"
        exit 1
    else
        echo "Removing stale PID file"
        rm -f "$PID_FILE"
    fi
fi

# 启动服务器
echo "Starting $SERVER_NAME..."
echo "Server binary: $SERVER_BIN"
echo "PID file: $PID_FILE"
echo "Log directory: $LOG_DIR"

cd "$PROJECT_DIR"

# 后台运行服务器
echo "Running in background..."
nohup "$SERVER_BIN" > "$LOG_DIR/server.out" 2>&1 &
SERVER_PID=$!

# 写入 PID 文件
echo "$SERVER_PID" > "$PID_FILE"
echo "PID written to $PID_FILE: $SERVER_PID"

# 等待服务器启动
sleep 2

# 检查服务器是否成功启动
if ps -p "$SERVER_PID" > /dev/null 2>&1; then
    echo "$SERVER_NAME started successfully (PID: $SERVER_PID)"
    echo "Log file: $LOG_DIR/server.out"
else
    echo "Failed to start $SERVER_NAME"
    echo "Check log file for details:"
    cat "$LOG_DIR/server.out"
    rm -f "$PID_FILE"
    exit 1
fi