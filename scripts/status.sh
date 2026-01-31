#!/bin/bash

# EasyChatServer 状态检查脚本

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# 配置变量
SERVER_NAME="EasyChatServer"
PID_FILE="$PROJECT_DIR/$SERVER_NAME.pid"

# 检查PID文件是否存在
if [ ! -f "$PID_FILE" ]; then
  echo "$SERVER_NAME is not running"
  exit 0
fi

# 读取PID
PID=$(cat "$PID_FILE")

# 检查进程是否存在
if ps -p "$PID" > /dev/null 2>&1; then
  echo "$SERVER_NAME is running (PID: $PID)"
  # 显示进程信息
  echo ""
  echo "Process Information:"
  ps -p "$PID" -o pid,ppid,%cpu,%mem,vsz,rss,tty,stat,start,time,comm
  # 显示网络连接
  echo ""
  echo "Network Connections:"
  netstat -anp 2>/dev/null | grep "$PID" | grep ESTABLISHED || echo "No active connections"

  exit 0
else
  echo "$SERVER_NAME is not running (stale PID file)"
  rm -f "$PID_FILE"
  exit 1
fi