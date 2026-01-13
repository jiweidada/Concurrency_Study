//
// Created by Administrator on 2026/1/13.
//
#include <iostream>
#include <thread>
#include <chrono>

// 一个简单的函数，将在新线程中运行
void thread_function(int id) {
    std::cout << "Thread " << id << " started\n";

    // 模拟工作
    for (int i = 0; i < 3; i++) {
        std::cout << "Thread " << id << " working...\n";
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