//
// Created by Administrator on 2026/1/14.
//
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

std::mutex mutex;

// 任务：计算数字的立方
void calculate_square(int n) {
    // 模拟计算时间
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "Thread " << std::this_thread::get_id()
                  << ": " << n << "^3 = " << n * n * n << std::endl;
    }

}

int main() {
    std::cout << "=== Thread Basics Demo ===\n";

    // 获取CPU核心数
    unsigned int cores = std::thread::hardware_concurrency();
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "Available CPU cores: " << cores << "\n\n";
    }


    // 创建多个线程
    std::vector<std::thread> threads;

    for (int i = 1; i <= 10; i++) {
        threads.push_back(std::thread(calculate_square, i));
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "Main thread: Created 5 threads\n";
        std::cout << "Main thread ID: " << std::this_thread::get_id() << "\n";
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 演示detach的使用
    std::cout << "\n=== Detach Demo ===\n";
    std::thread detached_thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "Detached thread finished\n";
    });

    // 分离线程
    detached_thread.detach();

    // 检查线程是否可join
    if (detached_thread.joinable()) {
        std::cout << "Thread is still joinable\n";
    } else {
        std::cout << "Thread is detached\n";
    }

    // 主线程等待一下，以便看到detached线程的输出
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::cout << "\nProgram finished successfully\n";
    return 0;
}
