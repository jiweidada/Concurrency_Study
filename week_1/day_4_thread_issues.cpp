//
// Created by Administrator on 2026/1/15.
//

//1. 学习如何向线程传递参数
//2. 观察多线程的常见问题
//3. 理解数据竞争的概念

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

//// 移动语义：转移所有权--lambda写法
//std::vector<int> data = {1, 2, 3};
//std::thread t([data = std::move(data)]() {
//    // data已被移动，原data不再可用
//});
//
////普通写法
//void process_data(std::vector<int> data){
//    for(auto i: data)
//    {
//        data[i]++;
//    }
//}
//std::thread t1(process_data,std::move(data));
//
//// 传递智能指针
//auto ptr = std::make_unique<int>(42);
//std::thread t([ptr = std::move(ptr)]() {
//    // 使用ptr
//});

std::mutex mutex;

// 全局共享变量 - 这是一个坏习惯，这里仅用于演示
//int shared_counter = 0;
std::atomic<int> shared_counter{0};

// 不安全的递增函数 - 有数据竞争
void unsafe_increment(int iterations) {
    for (int i = 0; i < iterations; i++) {
        shared_counter++;  // 这里会发生数据竞争！
    }
}

// 模拟一个缓慢的操作
void slow_operation(int id) {
    {
        std::lock_guard<std::mutex> lockGuard(mutex);
        std::cout << "Thread " << id << " started\n";
    }

    // 每个线程打印消息的顺序可能混乱
    for (int i = 0; i < 3; i++) {
        {
            std::lock_guard<std::mutex> lockGuard(mutex);
            std::cout << "Thread " << id << ": step " << i << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    {
        std::lock_guard<std::mutex> lockGuard(mutex);
        std::cout << "Thread " << id << " finished\n";
    }

}

// 传递不同参数类型的示例
void parameter_demo(int value, int& ref, const std::string& str) {
    value += 10;  // 修改不影响原始值
    ref += 10;    // 修改会影响原始值
    std::cout << str << " - Value: " << value << ", Ref: " << ref << "\n";
}

int main() {
    std::cout << "=== Thread Parameter Passing ===\n";

    int value = 5;
    int ref_value = 5;

    // 演示参数传递
    std::thread t1(parameter_demo, value, std::ref(ref_value), "Thread 1");
    t1.join();

    std::cout << "After thread: value = " << value
              << ", ref_value = " << ref_value << "\n\n";

    std::cout << "=== Observing Thread Order Issues ===\n";

    // 创建多个线程，观察执行顺序
    std::vector<std::thread> threads;
    for (int i = 1; i <= 3; i++) {
//        threads.push_back(std::thread(slow_operation, i));
        //在处理 线程这种不可拷贝的对象时，emplace_back 特别好用
        threads.emplace_back(slow_operation, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\n=== Demonstrating Data Race ===\n";

    // 重置计数器
    shared_counter = 0;

    // 创建两个线程同时修改共享变量
    std::thread worker1(unsafe_increment, 100000);
    std::thread worker2(unsafe_increment, 100000);

    worker1.join();
    worker2.join();

    // 期望值是200000，但由于数据竞争，实际值通常小于这个值
    std::cout << "Expected counter value: 200000\n";
    std::cout << "Actual counter value: " << shared_counter << "\n";
    std::cout << "Data race caused loss of: " << 200000 - shared_counter
              << " increments\n";

    return 0;
}