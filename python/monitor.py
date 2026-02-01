"""
服务器监控脚本
"""

import socket
import time
import threading
import json
import os
from datetime import datetime
from collections import deque

# 尝试导入psutil，如果未安装则提示
try:
    import psutil
    PSUTIL_AVAILABLE = True
except ImportError:
    PSUTIL_AVAILABLE = False
    print("⚠ 警告：psutil未安装，系统资源监控功能不可用")
    print("  安装命令：pip install psutil")

class ServerMonitor:
    """服务器监控类"""

    def __init__(self, host='localhost', port=8888, interval=5):
        """
        初始化监控器
        知识点：
        1. 参数默认值：interval默认5秒
        2. deque：双端队列，用于存储历史数据
        3. 限制大小：最多保存1000条记录
        """
        self.host = host
        self.port = port
        self.interval = interval  # 监控间隔（秒）
        self.running = False
        self.monitor_thread = None

        # 使用deque存储历史数据（最多1000条）
        # 知识点：deque是双端队列，比list性能更好
        self.metrics = deque(maxlen=1000)

        # 监控数据文件
        self.data_file = 'monitor_data.json'

        # 告警阈值
        self.thresholds = {
            'cpu_percent': 80.0,      # CPU使用率超过80%告警
            'memory_percent': 80.0,   # 内存使用率超过80%告警
            'disk_percent': 90.0,     # 磁盘使用率超过90%告警
            'response_time': 1000.0    # 响应时间超过1秒告警
        }

        # 告警回调函数
        self.alert_callback = None

    def start(self):
        """
        启动监控
        知识点：
        1. 线程启动：创建监控线程
        2. 守护线程：主线程退出时自动结束
        """
        if self.running:
            print("监控已在运行中")
            return

        self.running = True
        self.monitor_thread = threading.Thread(target=self._monitor_loop)
        self.monitor_thread.daemon = True
        self.monitor_thread.start()

        print(f"✓ 监控已启动，间隔: {self.interval}秒")
        print(f"  服务器: {self.host}:{self.port}")
        print(f"  数据文件: {self.data_file}")

    def stop(self):
        """
        停止监控
        知识点：
        1. 线程同步：设置running标志
        2. 等待线程结束：join方法
        3. 保存数据：停止前保存监控数据
        """
        if not self.running:
            return

        self.running = False
        if self.monitor_thread:
            self.monitor_thread.join(timeout=2)

        # 保存监控数据
        self._save_metrics()

        print("✓ 监控已停止")

    def _monitor_loop(self):
        """
        监控循环（在独立线程中运行）
        知识点：
        1. 无限循环：持续监控
        2. 定时采集：sleep控制采集间隔
        3. 异常处理：避免监控线程崩溃
        """
        while self.running:
            try:
                # 采集指标
                metric = self._collect_metrics()
                self.metrics.append(metric)

                # 输出监控信息
                self._print_metrics(metric)

                # 检查告警
                self._check_alerts(metric)

                # 定期保存数据（每10次保存一次）
                if len(self.metrics) % 10 == 0:
                    self._save_metrics()

                time.sleep(self.interval)

            except Exception as e:
                print(f"✗ 监控出错: {e}")
                time.sleep(self.interval)

    def _collect_metrics(self):
        """
        采集监控指标
        知识点：
        1. 时间戳：记录采集时间
        2. 服务器状态：检查服务器是否运行
        3. 系统资源：CPU、内存、磁盘使用情况
        4. 字典结构：使用字典存储多维度指标
        """
        timestamp = datetime.now().isoformat()

        # 服务器状态
        server_status = self._check_server_status()

        # 系统资源
        system_metrics = self._get_system_metrics()

        # 网络统计
        network_metrics = self._get_network_metrics()

        return {
            'timestamp': timestamp,
            'server': server_status,
            'system': system_metrics,
            'network': network_metrics
        }

    def _check_server_status(self):
        """
        检查服务器状态
        知识点：
        1. TCP连接测试：尝试连接服务器端口
        2. 响应时间测量：记录连接耗时
        3. 超时处理：设置连接超时时间
        """
        status = {
            'running': False,
            'response_time': -1,
            'error': None
        }

        try:
            # 尝试连接服务器
            start_time = time.time()
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(2)  # 2秒超时
            result = sock.connect_ex((self.host, self.port))
            end_time = time.time()

            if result == 0:
                status['running'] = True
                status['response_time'] = (end_time - start_time) * 1000  # 转换为毫秒
            else:
                status['error'] = f"连接失败，错误码: {result}"

            sock.close()

        except socket.timeout:
            status['error'] = "连接超时"
        except Exception as e:
            status['error'] = str(e)

        return status

    def _get_system_metrics(self):
        """
        获取系统资源指标
        知识点：
        1. psutil.cpu_percent：CPU使用率
        2. psutil.virtual_memory：内存使用情况
        3. psutil.disk_usage：磁盘使用情况
        4. 单位转换：字节转换为MB/GB
        """
        if not PSUTIL_AVAILABLE:
            return {
                'cpu_percent': -1,
                'memory_percent': -1,
                'disk_percent': -1,
                'note': 'psutil未安装'
            }

        try:
            # CPU使用率
            cpu_percent = psutil.cpu_percent(interval=0.1)

            # 内存使用情况
            memory = psutil.virtual_memory()

            # 磁盘使用情况
            disk = psutil.disk_usage('/')

            return {
                'cpu_percent': cpu_percent,
                'memory_percent': memory.percent,
                'memory_used_mb': memory.used / (1024 * 1024),
                'memory_total_mb': memory.total / (1024 * 1024),
                'disk_percent': disk.percent,
                'disk_used_gb': disk.used / (1024 * 1024 * 1024),
                'disk_total_gb': disk.total / (1024 * 1024 * 1024)
            }
        except Exception as e:
            return {
                'error': str(e)
            }

    def _get_network_metrics(self):
        """
        获取网络统计信息
        知识点：
        1. psutil.net_io_counters：网络I/O统计
        2. 流量统计：发送/接收字节数
        3. 连接数统计：统计TCP连接数
        """
        if not PSUTIL_AVAILABLE:
            return {
                'bytes_sent': -1,
                'bytes_recv': -1,
                'note': 'psutil未安装'
            }

        try:
            # 网络I/O统计
            net_io = psutil.net_io_counters()

            # TCP连接数
            tcp_connections = len(psutil.net_connections(kind='tcp'))

            return {
                'bytes_sent': net_io.bytes_sent,
                'bytes_recv': net_io.bytes_recv,
                'packets_sent': net_io.packets_sent,
                'packets_recv': net_io.packets_recv,
                'tcp_connections': tcp_connections
            }
        except Exception as e:
            return {
                'error': str(e)
            }

    def _print_metrics(self, metric):
        """
        打印监控指标
        知识点：
        1. 格式化输出：使用f-string格式化
        2. 状态标识：使用符号表示状态（✓/✗）
        3. 单位显示：自动选择合适的单位
        """
        print(f"\n{'='*60}")
        print(f"时间: {metric['timestamp']}")
        print(f"{'='*60}")

        # 服务器状态
        server = metric['server']
        status = "✓ 运行中" if server['running'] else "✗ 停止"
        print(f"服务器状态: {status}")
        if server['running']:
            print(f"  响应时间: {server['response_time']:.2f}ms")
        elif server['error']:
            print(f"  错误: {server['error']}")

        # 系统资源
        system = metric['system']
        if 'error' not in system:
            print(f"\n系统资源:")
            print(f"  CPU使用率: {system['cpu_percent']:.1f}%")
            print(f"  内存使用率: {system['memory_percent']:.1f}% ({system['memory_used_mb']:.0f}MB / {system['memory_total_mb']:.0f}MB)")
            print(f"  磁盘使用率: {system['disk_percent']:.1f}% ({system['disk_used_gb']:.1f}GB / {system['disk_total_gb']:.1f}GB)")

        # 网络统计
        network = metric['network']
        if 'error' not in network:
            print(f"\n网络统计:")
            print(f"  TCP连接数: {network['tcp_connections']}")
            print(f"  发送字节: {network['bytes_sent']:,}")
            print(f"  接收字节: {network['bytes_recv']:,}")

    def _check_alerts(self, metric):
        """
        检查告警条件
        知识点：
        1. 阈值比较：检查指标是否超过阈值
        2. 告警通知：调用回调函数
        3. 多条件检查：检查多个指标
        """
        alerts = []

        # 检查系统资源
        system = metric['system']
        if 'error' not in system:
            if system['cpu_percent'] > self.thresholds['cpu_percent']:
                alerts.append(f"CPU使用率过高: {system['cpu_percent']:.1f}%")

            if system['memory_percent'] > self.thresholds['memory_percent']:
                alerts.append(f"内存使用率过高: {system['memory_percent']:.1f}%")

            if system['disk_percent'] > self.thresholds['disk_percent']:
                alerts.append(f"磁盘使用率过高: {system['disk_percent']:.1f}%")

        # 检查服务器响应时间
        server = metric['server']
        if server['running'] and server['response_time'] > self.thresholds['response_time']:
            alerts.append(f"服务器响应时间过长: {server['response_time']:.2f}ms")

        # 检查服务器状态
        if not server['running']:
            alerts.append("服务器未运行")

        # 触发告警
        if alerts:
            for alert in alerts:
                print(f"⚠ 告警: {alert}")
                if self.alert_callback:
                    self.alert_callback(alert, metric)

    def _save_metrics(self):
        """
        保存监控数据到文件
        知识点：
        1. JSON序列化：将Python对象转换为JSON
        2. 文件操作：打开、写入、关闭文件
        3. 异常处理：处理文件操作异常
        """
        try:
            # 将deque转换为list
            data = list(self.metrics)

            # 写入JSON文件
            with open(self.data_file, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)

            print(f"✓ 监控数据已保存: {len(data)}条记录")

        except Exception as e:
            print(f"✗ 保存监控数据失败: {e}")

    def load_metrics(self):
        """
        从文件加载监控数据
        知识点：
        1. JSON反序列化：将JSON转换为Python对象
        2. 文件存在性检查：避免文件不存在时报错
        """
        if not os.path.exists(self.data_file):
            return []

        try:
            with open(self.data_file, 'r', encoding='utf-8') as f:
                data = json.load(f)

            # 转换为deque
            self.metrics = deque(data, maxlen=1000)
            print(f"✓ 监控数据已加载: {len(self.metrics)}条记录")

            return data

        except Exception as e:
            print(f"✗ 加载监控数据失败: {e}")
            return []

    def set_alert_callback(self, callback):
        """
        设置告警回调函数
        知识点：
        1. 回调模式：外部函数处理告警
        2. 参数传递：告警消息和监控数据
        """
        self.alert_callback = callback

    def set_threshold(self, name, value):
        """
        设置告警阈值
        知识点：
        1. 参数验证：检查阈值名称是否有效
        2. 动态配置：运行时修改阈值
        """
        if name in self.thresholds:
            self.thresholds[name] = value
            print(f"✓ 阈值已更新: {name} = {value}")
        else:
            print(f"✗ 无效的阈值名称: {name}")
            print(f"  可用阈值: {', '.join(self.thresholds.keys())}")

    def get_statistics(self):
        """
        获取统计信息
        知识点：
        1. 数据分析：计算平均值、最大值等
        2. 数据过滤：只分析有效数据
        """
        if not self.metrics:
            return None

        # 过滤有效数据
        valid_metrics = [m for m in self.metrics if m['server']['running']]

        if not valid_metrics:
            return None

        # 计算响应时间统计
        response_times = [m['server']['response_time'] for m in valid_metrics]
        avg_response = sum(response_times) / len(response_times)
        max_response = max(response_times)
        min_response = min(response_times)

        # 计算系统资源统计
        cpu_values = [m['system']['cpu_percent'] for m in valid_metrics if 'cpu_percent' in m['system']]
        memory_values = [m['system']['memory_percent'] for m in valid_metrics if 'memory_percent' in m['system']]

        avg_cpu = sum(cpu_values) / len(cpu_values) if cpu_values else 0
        avg_memory = sum(memory_values) / len(memory_values) if memory_values else 0

        return {
            'total_samples': len(self.metrics),
            'valid_samples': len(valid_metrics),
            'response_time': {
                'avg': avg_response,
                'max': max_response,
                'min': min_response
            },
            'system': {
                'avg_cpu': avg_cpu,
                'avg_memory': avg_memory
            }
        }

# 告警回调示例
def alert_handler(alert, metric):
    """告警处理函数"""
    timestamp = metric['timestamp']
    print(f"\n📧 告警通知 [{timestamp}]: {alert}")
    # 这里可以添加邮件、短信等通知方式

# 主程序
def main():
    """主程序"""
    print("=" * 60)
    print("EasyChat服务器监控")
    print("=" * 60)

    # 创建监控器
    monitor = ServerMonitor('localhost', 8888, interval=10)

    # 设置告警回调
    monitor.set_alert_callback(alert_handler)

    # 加载历史数据
    monitor.load_metrics()

    # 启动监控
    monitor.start()

    try:
        # 主循环
        while True:
            print("\n命令 (help查看帮助): ", end='', flush=True)
            cmd = input().strip().lower()

            if cmd == 'help':
                print("可用命令:")
                print("  stats    - 显示统计信息")
                print("  save     - 保存监控数据")
                print("  load     - 加载监控数据")
                print("  clear    - 清空监控数据")
                print("  quit     - 退出")

            elif cmd == 'stats':
                stats = monitor.get_statistics()
                if stats:
                    print(f"\n统计信息:")
                    print(f"  总样本数: {stats['total_samples']}")
                    print(f"  有效样本数: {stats['valid_samples']}")
                    print(f"  平均响应时间: {stats['response_time']['avg']:.2f}ms")
                    print(f"  最大响应时间: {stats['response_time']['max']:.2f}ms")
                    print(f"  最小响应时间: {stats['response_time']['min']:.2f}ms")
                    print(f"  平均CPU使用率: {stats['system']['avg_cpu']:.1f}%")
                    print(f"  平均内存使用率: {stats['system']['avg_memory']:.1f}%")
                else:
                    print("暂无有效数据")

            elif cmd == 'save':
                monitor._save_metrics()

            elif cmd == 'load':
                monitor.load_metrics()

            elif cmd == 'clear':
                monitor.metrics.clear()
                print("监控数据已清空")

            elif cmd == 'quit':
                break

            else:
                print("未知命令，输入help查看帮助")

    except KeyboardInterrupt:
        print("\n")

    # 停止监控
    monitor.stop()
    print("监控脚本已退出")

if __name__ == "__main__":
    main()