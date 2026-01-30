//
// Created by Cando on 2026/1/29.
//

#ifndef EASYCHATSERVER_THREADPOOL_H
#define EASYCHATSERVER_THREADPOOL_H

#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

namespace easychat{
    //std::function 可以存储任意可调用对象，包括函数指针、lambda 表达式等
    using Task = std::function<void()>;
    //线程池类
    class ThreadPool{
    public:
        explicit ThreadPool(size_t thread_count=std::thread::hardware_concurrency());
        ~ThreadPool();
        // 禁止拷贝和赋值
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        // 提交任务到线程池
        // 使用模板和 std::future 实现异步任务执行和结果获取
        template<typename F,typename... Args> auto submit(F&& f,Args&&...args)->
        std::future<decltype(f(args...))>{
            // 推导返回类型
            using ReturnType = decltype(f(args...));
            // 创建 packaged_task,将任务和返回值绑定
            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                    std::bind(std::forward<F>(f),
                    std::forward<Args>(args)...)
                    );
            // 获取 future 对象，用于获取任务执行结果
            std::future<ReturnType> result = task->get_future();
            {
                // 加锁保护任务队列
                std::lock_guard<std::mutex> lock(queue_mutex_);
                // 线程池停止抛出异常
                if (stop_){
                    throw std::runtime_error("submit on stopped ThreadPool");
                }
                // 将任务添加到队列
                tasks_.emplace([task](){(*task)();});
            }
            // 通知工作线程有新任务
            condition_.notify_one();
            return result;
        }
        // 获取线程数
        size_t getThreadCount() const{
            return threads_.size();
        }
        // 获取任务队列
        size_t getTaskQueueSize() const{
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return tasks_.size();
        }
    private:
        // 工作线程函数
        void worker();
        // 工作线程组
        std::vector<std::thread> threads_;
        // 任务队列
        std::queue<Task> tasks_;
        // 任务队列互斥锁
        mutable std::mutex queue_mutex_;
        // 条件变量用于线程同步
        std::condition_variable condition_;
        //原子变量，标记线程池是否停止
        std::atomic<bool> stop_;
    };
}

#endif //EASYCHATSERVER_THREADPOOL_H
