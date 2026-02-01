"""
监控使用示例
"""
import time
from monitor import ServerMonitor

def main():
    # 创建监控器
    monitor = ServerMonitor('localhost', 8888, interval=5)

    # 设置告警回调
    def on_alert(alert, metric):
        print(f"📧 告警: {alert}")
        # 可以在这里添加邮件、短信等通知

    monitor.set_alert_callback(on_alert)

    # 启动监控
    monitor.start()

    # 运行60秒
    try:
        time.sleep(60)
    except KeyboardInterrupt:
        pass

    # 停止监控
    monitor.stop()

    # 显示统计信息
    stats = monitor.get_statistics()
    if stats:
        print(f"\n统计信息:")
        print(f"平均响应时间: {stats['response_time']['avg']:.2f}ms")
        print(f"平均CPU使用率: {stats['system']['avg_cpu']:.1f}%")

if __name__ == "__main__":
    main()