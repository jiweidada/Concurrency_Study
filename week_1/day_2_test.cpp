//
// Created by Administrator on 2026/1/13.
//
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex cout_mutex;

// 一个简单的函数，将在新线程中运行
void thread_function(int id) {
    std::thread::id current_thread_id = std::this_thread::get_id();

    {
        //std::cout在多线程环境下不是线程安全的 使用互斥锁保护输出
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << id << " started\n";
    }

    // 模拟工作
    for (int i = 0; i < 3; i++) {

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << id  << " working..."
                      << "Current Thread id:" << current_thread_id << "\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Thread " << id << " finished\n";
}

int main() {
    std::cout << "Main thread started\n";

    // 创建并启动线程
    std::thread t1(thread_function, 1);
    std::thread t2(thread_function, 2);

    std::cout << "Main thread waiting for threads to finish...\n";

    // 等待线程完成
    t1.join();
    t2.join();

    std::cout << "Main thread finished\n";
    return 0;
}