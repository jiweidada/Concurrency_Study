#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <functional>
#include <iostream>
#include <atomic>
#include <chrono>

// =========================
// 线程安全队列：支持 close()
// =========================
template<typename T>
class SafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cv_not_full;
    std::condition_variable cv_not_empty;

    size_t max_size;
    bool closed_ = false; // 新增：队列是否关闭

public:
    SafeQueue(size_t size) : max_size(size) {}

    // 关闭队列：唤醒所有等待线程，让它们有机会退出
    void close() {
        std::lock_guard<std::mutex> lock(mtx_);
        closed_ = true;
        cv_not_empty.notify_all();
        cv_not_full.notify_all();
    }

    // 生产数据：如果队列已关闭，直接返回 false 表示失败
    bool produce(T value) {
        std::unique_lock<std::mutex> lock(mtx_);

        // 等待：队列未满 或 队列已关闭
        cv_not_full.wait(lock, [this]() {
            return closed_ || queue_.size() < max_size;
        });

        if (closed_) return false; // 关闭后不再接受新任务

        queue_.push(std::move(value));
        lock.unlock();

        cv_not_empty.notify_one();
        return true;
    }

    // 消费数据：阻塞等待
    // 返回值：true 表示拿到数据；false 表示队列关闭且已空 -> 该退出了
    bool consume(T& value) {
        std::unique_lock<std::mutex> lock(mtx_);

        // 等待：队列非空 或 队列关闭
        cv_not_empty.wait(lock, [this]() {
            return closed_ || !queue_.empty();
        });

        // 如果队列关闭且没有数据了 -> 告诉调用者退出
        if (queue_.empty()) {
            return false;
        }

        value = std::move(queue_.front());
        queue_.pop();

        lock.unlock();
        cv_not_full.notify_one();
        return true;
    }
};

// =========================
// 安全线程池（可析构）
// =========================
class ThreadPool {
private:
    using Task = std::function<void()>;

    std::vector<std::thread> workers_;
    SafeQueue<Task> tasks_;
    std::atomic<bool> stop_{false}; // 新增：停止标志（辅助理解用）

public:
    ThreadPool(size_t numThreads)
            : tasks_(100)
    {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this]() {
                while (true) {
                    Task task;

                    // 如果返回 false：表示队列关闭且已空 -> 安全退出线程
                    if (!tasks_.consume(task)) {
                        break;
                    }

                    // 正常执行任务
                    if (task) task();
                }
            });
        }
    }

    // 提交任务：如果线程池已停止/队列已关闭，会提交失败
    bool enqueue(Task task) {
        if (stop_.load()) return false;
        return tasks_.produce(std::move(task));
    }

    // 析构：通知线程退出 + join 等待收尾
    ~ThreadPool() {
        stop_.store(true);
        tasks_.close(); // 关键：唤醒所有阻塞在 consume() 的工人

        // join：等每个工人线程正常退出（不再 detach）
        for (auto& t : workers_) {
            if (t.joinable()) t.join();
        }
    }
};

// =========================
// 测试
// =========================
int main() {
    ThreadPool pool(4);
    std::mutex mtx;

    for (int i = 0; i < 80; ++i) {
        pool.enqueue([i, &mtx] {
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "Task " << i << " is running on thread "
                          << std::this_thread::get_id() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
    }

    std::cout << "main is over" << std::endl;
    // main 不用刻意 sleep 很久了：
    // 当 main 结束，pool 析构会 join 等任务跑完再退出
    return 0;
}
