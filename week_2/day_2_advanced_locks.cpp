//
// Created by Administrator on 2026/1/16.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>  // C++17
#include <vector>
#include <chrono>



// 1. 递归互斥锁示例
class RecursiveCounter {
private:
    std::recursive_mutex rmutex;
    int count = 0;

public:
    void increment() {
        std::lock_guard<std::recursive_mutex> lock(rmutex);
        count++;
    }

    // 递归函数可以多次获取同一个锁
    void double_increment() {
        std::lock_guard<std::recursive_mutex> lock(rmutex);
        increment();  // 这里会再次获取锁
        increment();  // 这里会再次获取锁
    }

    int get_count() {
        std::lock_guard<std::recursive_mutex> lock(rmutex);
        return count;
    }
};

std::mutex mtx;
// 2. 共享锁（读写锁）示例
class ThreadSafeData {
private:
    mutable std::shared_mutex mutex;  // mutable 允许const函数加锁
    std::vector<int> data;

public:
    // 写操作：使用排他锁
    void add_value(int value) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        data.push_back(value);
        {
            std::lock_guard<std::mutex> lockGuard(mtx);
        }
        std::cout << "Added value: " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟耗时操作
    }

    // 读操作：使用共享锁（允许多个线程同时读）
    int get_sum() const {
        std::shared_lock<std::shared_mutex> lock(mutex);
        int sum = 0;
        for (int val : data) {
            sum += val;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));  // 模拟耗时操作
        return sum;
    }

    // 读操作：使用共享锁
    size_t get_size() const {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return data.size();
    }
};

// 3. 死锁演示
void deadlock_demo() {
    std::mutex mutex1, mutex2;

    auto task1 = [&]() {
        std::lock_guard<std::mutex> lock1(mutex1);
        std::cout << "Task1 locked mutex1\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 增加死锁概率

        std::lock_guard<std::mutex> lock2(mutex2);  // 等待mutex2
        std::cout << "Task1 locked mutex2\n";
    };

    auto task2 = [&]() {
        std::lock_guard<std::mutex> lock2(mutex2);
        std::cout << "Task2 locked mutex2\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 增加死锁概率

        std::lock_guard<std::mutex> lock1(mutex1);  // 等待mutex1
        std::cout << "Task2 locked mutex1\n";
    };

    std::thread t1(task1);
    std::thread t2(task2);

    t1.join();
    t2.join();
    std::cout << "No deadlock occurred (lucky!)\n";
}

// 4. 死锁预防：使用std::lock一次性锁定多个互斥量
void deadlock_prevention_demo() {
    std::mutex mutex1, mutex2;

    auto safe_task1 = [&]() {

//        这一行代码 std::scoped_lock lock(mutex1, mutex2); 背后其实发生了三件大事：
//
//        自动推导 (Deduced): 编译器自动推导出锁的类型（不需要写 <std::mutex, std::mutex>）。
//        原子性算法 (Deadlock Avoidance): 它内部会自动调用类似 std::lock 的算法，确保“要么两个都拿到，要么一个都不拿”，不用担心死锁。
//        RAII 管理: 出了作用域 { } 自动解锁。
        std::scoped_lock lock(mutex1, mutex2);

        std::cout << "SafeTask1 locked both mutexes\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "SafeTask1 finished\n";
    };

    auto safe_task2 = [&]() {

        std::scoped_lock lock(mutex1, mutex2);
        std::cout << "SafeTask2 locked both mutexes\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "SafeTask2 finished\n";
    };

    std::thread t1(safe_task1);
    std::thread t2(safe_task2);

    t1.join();
    t2.join();
    std::cout << "Both tasks completed safely\n";
}

// 5. 超时锁：try_lock_for
void timeout_lock_demo() {
    std::timed_mutex tmutex;  // 支持超时的互斥锁

    auto task_with_timeout = [&](int id) {
        std::cout << "Task " << id << " trying to lock...\n";

        // 尝试获取锁，最多等待100毫秒
        if (tmutex.try_lock_for(std::chrono::milliseconds(100))) {
            std::cout << "Task " << id << " got the lock\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            tmutex.unlock();
            std::cout << "Task " << id << " released the lock\n";
        } else {
            std::cout << "Task " << id << " failed to get lock (timeout)\n";
        }
    };

    // 第一个线程获取锁并持有较长时间
    std::thread t1([&]() {
        tmutex.lock();
        std::cout << "Task 1 got lock and will hold for 300ms\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        tmutex.unlock();
    });

    // 给第一个线程一点时间获取锁
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // 其他线程尝试获取锁
    std::thread t2(task_with_timeout, 2);
    std::thread t3(task_with_timeout, 3);

    t1.join();
    t2.join();
    t3.join();
}

int main() {
    std::cout << "=== Advanced Locks Demo ===\n\n";

    // 1. 递归锁演示
    std::cout << "1. Recursive Mutex Example:\n";
    RecursiveCounter rc;
    rc.double_increment();
    std::cout << "Count after double_increment: " << rc.get_count() << "\n\n";

    // 2. 共享锁演示
    std::cout << "2. Shared Mutex (Read-Write Lock) Example:\n";
    ThreadSafeData tsd;

    std::thread writer([&tsd]() {
        for (int i = 1; i <= 3; i++) {
            tsd.add_value(i * 10);
        }
    });


    std::thread reader1([&tsd]() {
        for (int i = 0; i < 3; i++) {
            int tsd_sum = tsd.get_sum();
            int tsd_size = tsd.get_size();
            {
                std::lock_guard<std::mutex> lockGuard(mtx);
                std::cout << "Reader1: sum = " << tsd_sum
                          << ", size = " << tsd_size << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    });

    std::thread reader2([&tsd]() {
        for (int i = 0; i < 3; i++) {
            int tsd_sum = tsd.get_sum();
            int tsd_size = tsd.get_size();
            {
                std::lock_guard<std::mutex> lockGuard(mtx);
                std::cout << "Reader2: sum = " << tsd_sum
                          << ", size = " << tsd_size << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(120));
        }
    });

    writer.join();
    reader1.join();
    reader2.join();
    std::cout << "\n";

    // 3. 死锁预防演示（先运行安全的版本）
    std::cout << "3. Deadlock Prevention Example:\n";
    deadlock_prevention_demo();
    std::cout << "\n";

//    // 3. 死锁演示
//    std::cout << "3. Deadlock  Example:\n";
//    deadlock_demo();
//    std::cout << "\n";


    // 4. 超时锁演示
    std::cout << "4. Timeout Lock Example:\n";
    timeout_lock_demo();

    return 0;
}