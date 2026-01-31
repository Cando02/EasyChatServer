#!/bin/bash

#EasyChatServer停止脚本

#获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# 配置变量
SERVER_NAME="EasyChatServer"
PID_FILE="$PROJECT_DIR/$SERVER_NAME.pid"

# 检查PID文件是否存在
if [ ! -f "$PID_FILE" ]; then
  echo "$SERVER_NAME is not running (no PID file found)"
  exit 0
fi

# 读取PID
PID=$(cat "$PID_FILE")

# 检查进程是否存在
if  ! ps -p "$PID" > /dev/null 2>&1 ; then
    echo "$SERVER_NAME is not running (stale PID file)"
    rm -f "$PID_FILE"
    exit 0
fi

# 停止服务器
echo "Stopping $SERVER_NAME (PID: $PID)..."

# 发送SIGTERM终止信号
kill -TERM "$PID"

# 等待进程退出
TIMEOUT=10
COUNT=0
while ps -p "$PID" > /dev/null 2>&1 && [ $COUNT -lt $TIMEOUT ]; do
  sleep 1
  COUNT=$((COUNT+1))
done

# 检查进程是否退出
if ps -p "$PID" > /dev/null 2>&1; then
  echo "$SERVER_NAME did not stop gracefully, forcing..."
  # 强制终止
  kill -KILL "$PID"
  sleep 1
  if ps -p "$PID" > /dev/null 2>&1; then
    echo "Failed to stop $SERVER_NAME"
    exit 1
  fi
fi

#删除PID文件
rm -f "$PID_FILE"

echo "$SERVER_NAME stopped successfully"
