//
// Created by 15728 on 2026/2/20.
//

#include <iostream>
#include <thread>
#include <vector>
#include <random> // 新增：C++11 随机数库
#include "ThreadSafeLRUCache.h"

// 仅在 Windows 下包含 windows.h 并设置控制台编码
#ifdef _WIN32
#include <windows.h>
#endif

// 模拟一个“读多写少”的场景
void read_heavy_task(ThreadSafeLRUCache<int, int>& cache, int thread_id) {

    // [修改点 1] 每个线程拥有自己的随机数生成器
    // 使用 std::random_device 作为种子（注意：某些平台上 random_device 可能不是真随机，但用于测试足够）
    // 为了演示简单，我们用 thread_id 作为种子，保证每个线程序列不同且可复现
    std::mt19937 gen(thread_id + std::random_device{}());

    // [修改点 2] 设定分布：我们要生成 0 到 99 的整数，用来做概率判断
    std::uniform_int_distribution<> prob_dist(0, 99);
    // 设定分布：生成 Key
    std::uniform_int_distribution<> hot_key_dist(0, 4);  // 热点 Key: 0-4
    std::uniform_int_distribution<> all_key_dist(0, 9);  // 全部 Key: 0-9

    for (int i = 0; i < 1000; ++i) {
        // 80% 的概率访问 0-4 (热点数据)，20% 访问 5-9 (冷门数据)
        int key = (prob_dist(gen) < 80) ? hot_key_dist(gen) : all_key_dist(gen);

        try {
            cache.get(key);
        } catch (...) {
            // 未命中，不管它
        }
    }
}

void writer_task(ThreadSafeLRUCache<int, int>& cache) {
    // 只写入少量数据
    for (int i = 0; i < 10; ++i) {
        cache.put(i, i * 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {

    // 仅在 Windows 下执行控制台编码设置
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    ThreadSafeLRUCache<int, int> cache(10);

    // 1. 先预热缓存（写入一些数据）
    std::cout << "正在预热缓存..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        cache.put(i, i * 100);
    }




    // 2. 开始并发测试
    std::cout << "启动多线程读写测试 (Read-Heavy)..." << std::endl;
    cache.reset_stats();

    std::vector<std::thread> threads;

    // 启动 1 个写线程
    threads.emplace_back(writer_task, std::ref(cache));

    // 启动 10 个读线程
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(read_heavy_task, std::ref(cache), i);
    }

    // 等待结束
    for (auto& t : threads) {
        t.join();
    }

    // 3. 打印统计结果
    std::cout << "\n=== 测试结果 ===" << std::endl;
    std::cout << "最终缓存大小: " << cache.size() << std::endl;
    std::cout << "总访问次数: " << (cache.get_hit_count() + cache.get_miss_count()) << std::endl;
    std::cout << "命中 (Hit): " << cache.get_hit_count() << std::endl;
    std::cout << "未命中 (Miss): " << cache.get_miss_count() << std::endl;
    std::cout << "** 缓存命中率: " << (cache.get_hit_rate() * 100) << "% **" << std::endl;

    return 0;
}