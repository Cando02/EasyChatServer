//
// Created by Cando on 2026/1/29.
//
#include "../../include/threadpool/threadpool.h"
#include <iostream>

namespace easychat{
    ThreadPool::ThreadPool(size_t thread_count) : stop_(false){
        // 创建线程
        for (size_t i = 0;i<thread_count;++i){
            threads_.emplace_back([this]{ this->worker();});
        }
        std::cerr<<"ThreadPool created with: "<<thread_count<<" threads"<<std::endl;
    }
    ThreadPool::~ThreadPool(){
        // 停止线程池
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }

        // 唤醒所有等待线程
        condition_.notify_all();
        // 等待工作线程结束
        for (std::thread& thread: threads_){
            if (thread.joinable()){
                // 等待thread执行完毕
                thread.join();
            }
        }
        std::cout<<"ThreadPool destroyed"<<std::endl;
    }
    void ThreadPool::worker() {
        while (true){
            Task task;
            // 知识：块作用域，变量只存在于这个块中
            {
                // 等待任务或停止信号
                std::unique_lock<std::mutex> lock(queue_mutex_);
                // 等待条件变量
                condition_.wait(lock,[this]{
                    return this->stop_ || !this->tasks_.empty();
                });
                // 线程池停止且任务队列为空，退出线程
                if (this->stop_ && this->tasks_.empty()){
                    return;
                }
                // 从队列中取出任务
                task = std::move(this->tasks_.front());
                this->tasks_.pop();
            }
            // 执行任务
            try{
                task();
            }catch (const std::exception& e){
                std::cerr<<"Task execution error: "<<e.what()<<std::endl;
            }
        }
    }
}