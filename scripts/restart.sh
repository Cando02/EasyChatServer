#!/bin/bash

# EasyChatServer 重启脚本

#获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

#停止服务器
echo "Restarting EasyChatServer..."
"$SCRIPT_DIR/stop.sh"

#等待服务器停止
sleep 2

#启动服务器
"$SCRIPT_DIR/start.sh"