//
// Created by Administrator on 2026/2/5.
//

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <random>

// 1. 基本条件变量示例
void basic_condition_variable() {
    std::cout << "=== Basic Condition Variable ===\n";

    std::mutex mtx;
    std::condition_variable cv;
    bool data_ready = false;
    int shared_data = 0;

    // 消费者线程：等待数据就绪
    std::thread consumer([&mtx,&data_ready,&shared_data,&cv]() {
        std::cout << "Consumer: waiting for data...\n";

        std::unique_lock<std::mutex> lock(mtx);
        // 使用谓词防止虚假唤醒
        cv.wait(lock, [&data_ready]() { return data_ready; });

        std::cout << "Consumer: received data = " << shared_data << "\n";
    });

    // 生产者线程：准备数据并通知
    std::thread producer([&mtx,&shared_data,&data_ready,&cv]() {
        std::cout << "Producer: preparing data...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));

        {
            std::lock_guard<std::mutex> lock(mtx);
            shared_data = 42;
            data_ready = true;
            std::cout << "Producer: data is ready\n";
        }

        cv.notify_one();  // 通知消费者
    });

    consumer.join();
    producer.join();
    std::cout << "\n";
}

// 2. 生产者-消费者模式（有界缓冲区）
class BoundedBuffer {
private:
    std::queue<int> buffer;
    size_t max_size;

    mutable std::mutex mtx;  // 关键修改：添加 mutable
    std::condition_variable cv_not_full;
    std::condition_variable cv_not_empty;

public:
    BoundedBuffer(size_t size) : max_size(size) {}

    void produce(int value) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_not_full.wait(lock, [this]()
        {
            return buffer.size() < max_size;
        });

        buffer.push(value);
        std::cout << "Produced: " << value
                  << " (buffer size: " << buffer.size() <<" current thread:" << std::this_thread::get_id() << ")\n";

        lock.unlock();
        cv_not_empty.notify_one();
    }

    int consume() {
        std::unique_lock<std::mutex> lock(mtx);
        cv_not_empty.wait(lock, [this]() {
            return !buffer.empty();
        });

        int value = buffer.front();
        buffer.pop();
        std::cout << "Consumed: " << value
                  << " (buffer size: " << buffer.size() <<" current thread:" << std::this_thread::get_id()<< ")\n";

        lock.unlock();
        cv_not_full.notify_one();
        return value;
    }

    size_t size() const {  // 现在可以正常工作
        std::lock_guard<std::mutex> lock(mtx);
        return buffer.size();
    }
};

void producer_consumer_demo() {
    std::cout << "=== Producer-Consumer Pattern ===\n";

    BoundedBuffer buffer(5);  // 缓冲区大小为5

    // 启动多个生产者和消费者
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // 创建3个生产者
    for (int i = 0; i < 3; i++) {
        producers.emplace_back([&buffer, i]() {
            std::random_device rd;  // 1. 随机数种子（真随机数，来自硬件）
            std::mt19937 gen(rd()); // 2. 生成器（梅森旋转算法，生成高质量伪随机数）
            std::uniform_int_distribution<> dis(100, 500); // 3. 分布范围（限制在 100 到 500 之间）

            for (int j = 0; j < 4; j++) {
                int value = i * 100 + j;
                buffer.produce(value);
                std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
            }
        });
    }

    // 创建2个消费者
    for (int i = 0; i < 2; i++) {
        consumers.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(150, 600);

            for (int j = 0; j < 6; j++) {
                buffer.consume();
                std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
            }
        });
    }

    // 等待所有线程完成
    for (auto& p : producers) {
        p.join();
    }
    for (auto& c : consumers) {
        c.join();
    }

    std::cout << "Final buffer size: " << buffer.size() << "\n\n";
}

// 3. 等待超时：wait_for 和 wait_until
void timeout_wait_demo() {
    std::cout << "=== Timeout Wait ===\n";

    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;

    std::thread worker([&mtx,&ready,&cv]() {
        std::cout << "Worker: starting long task...\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));

        {
            std::lock_guard<std::mutex> lock(mtx);
            ready = true;
        }
        cv.notify_one();
        std::cout << "Worker: task completed, notified\n";
    });

    std::thread waiter([&]() {
        std::cout << "Waiter: waiting with timeout...\n";

        std::unique_lock<std::mutex> lock(mtx);

        // 等待最多2秒
        auto status = cv.wait_for(lock,
                                  std::chrono::seconds(2),
                                  [&ready]() { return ready; }
        );

        if (status) {
            std::cout << "Waiter: condition met!\n";
        } else {
            std::cout << "Waiter: timeout! Condition not met in time.\n";
        }
    });

    worker.join();
    waiter.join();
    std::cout << "\n";
}

// 4. 条件变量的常见陷阱
void condition_variable_pitfalls() {
    std::cout << "=== Common Pitfalls ===\n";

    std::mutex mtx;
    std::condition_variable cv;
    bool flag = false;

    // 陷阱1：丢失通知（通知在等待之前发生）
    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
            std::lock_guard<std::mutex> lock(mtx);
            flag = true;
        }
        cv.notify_one();
        std::cout << "Notifier: sent notification\n";
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));  // 确保通知先发生

    std::thread waiter([&]() {
        std::unique_lock<std::mutex> lock(mtx);

        // 这里会永远等待，因为通知已经发生过了
        std::cout << "Waiter: waiting for notification...\n";
        cv.wait(lock, [&]() { return flag; });

        std::cout << "Waiter: finally got notification\n";
    });
    //它甚至看似顺利结束了，但结果却是错的。这就好比接力赛中，第二棒选手还没拿到接力棒就开始跑了，虽然他也跑到了终点，但这个成绩是无效的
    notifier.join();
    waiter.join();

    std::cout << "Lesson: Always check condition before waiting\n\n";
}

int main() {
    std::cout << "=== Condition Variable Demo ===\n\n";

//    basic_condition_variable();
//    producer_consumer_demo();
//    timeout_wait_demo();
    condition_variable_pitfalls();

    return 0;
}