//
// Created by Administrator on 2026/1/16.
//
#include <iostream>
#include <thread>
#include <mutex>

// 全局共享变量
int shared_counter = 0;
std::mutex counter_mutex;

// 不安全的递增 - 有数据竞争
void unsafe_increment(int iterations) {
    for (int i = 0; i < iterations; i++) {
        shared_counter++;  // 数据竞争！
    }
}

// 安全的递增 - 使用互斥锁保护
void safe_increment(int iterations) {
    for (int i = 0; i < iterations; i++) {
        // 使用lock_guard自动管理锁
        std::lock_guard<std::mutex> lock(counter_mutex);
        shared_counter++;
    }
}

// 手动使用mutex的lock/unlock
void manual_mutex_increment(int iterations) {
    for (int i = 0; i < iterations; i++) {
        counter_mutex.lock();  // 手动加锁
        shared_counter++;
        counter_mutex.unlock();  // 手动解锁
    }
}

void demo_unsafe() {
    std::cout << "=== Unsafe Increment (Data Race) ===\n";
    shared_counter = 0;

    std::thread t1(unsafe_increment, 10000000);
    std::thread t2(unsafe_increment, 10000000);

    t1.join();
    t2.join();

    std::cout << "Expected: 20000000\n";
    std::cout << "Actual: " << shared_counter << "\n";
    std::cout << "Lost " << 20000000 - shared_counter << " increments\n\n";
}

void demo_safe() {
    std::cout << "=== Safe Increment (with std::lock_guard) ===\n";
    shared_counter = 0;

    std::thread t1(safe_increment, 10000000);
    std::thread t2(safe_increment, 10000000);

    t1.join();
    t2.join();

    std::cout << "Expected: 20000000\n";
    std::cout << "Actual: " << shared_counter << "\n";
    std::cout << "Perfect! No data race.\n\n";
}

void demo_manual() {
    std::cout << "=== Manual Mutex Lock/Unlock ===\n";
    shared_counter = 0;

    std::thread t1(manual_mutex_increment, 10000000);
    std::thread t2(manual_mutex_increment, 10000000);

    t1.join();
    t2.join();

    std::cout << "Expected: 20000000\n";
    std::cout << "Actual: " << shared_counter << "\n";
    std::cout << "Also safe, but riskier than lock_guard\n\n";
}

// 演示锁的粒度问题
void demo_lock_granularity() {
    std::cout << "=== Lock Granularity Example ===\n";

    std::mutex mtx;
    int data1 = 0;
    int data2 = 0;

    // 粗粒度锁：一次锁定保护多个操作
    auto coarse_grained = [&data1,&data2,&mtx]() {
        std::lock_guard<std::mutex> lock(mtx);
        data1++;
        // 模拟一些工作
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        data2++;
    };

    // 细粒度锁：只锁定必要的部分
    auto fine_grained = [&data1,&data2,&mtx]() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            data1++;
        }
        // 这里可以做不需要锁的工作
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        {
            std::lock_guard<std::mutex> lock(mtx);
            data2++;
        }
    };

    std::cout << "Lock granularity affects performance and concurrency.\n";
    std::cout << "Coarse-grained: less locking overhead, less concurrency\n";
    std::cout << "Fine-grained: more locking overhead, more concurrency\n\n";
}

int main() {
    // 注意：由于数据竞争，unsafe版本的结果可能每次都不同
    demo_unsafe();
    demo_safe();
    demo_manual();
    demo_lock_granularity();

    return 0;
}