//
// Created by Administrator on 2026/2/13.
//

//1. 核心功能需求
//你需要实现一个函数 parallel_merge_sort，它能够利用多核 CPU 加速排序过程。
//算法选择：归并排序 (Merge Sort)。因为它极其适合通过 std::async 进行任务分割。
//输入：一个巨大的 std::vector<int>（建议 100万 ~ 1000万 数据量）。
//输出：数据被原地排序（从小到大）。

#include <iostream>
#include <vector>
#include <algorithm> // std::sort, std::inplace_merge, std::is_sorted
#include <future>    // std::async, std::future
#include <random>    // 生成随机数
#include <chrono>    // 计时器
#include <thread>    // 获取硬件并发数

#ifdef _WIN32
#include <windows.h>
#endif

// NOLINTNEXTLINE(misc-no-recursion)
template <typename RandomIt>
void parallel_merge_sort(RandomIt begin, RandomIt end)
{
    //1. 获取数据长度
    auto len = std::distance(begin,end);

    // 2. 【递归终止条件】
    // 如果数据太少（比如少于 1000 个），开启线程的开销反而比排序本身大。
    // 这时候直接用普通的 std::sort 就行
    if (len < 1000)
    {
        std::sort(begin,end);
        return;
    }

    // 3. 【任务分割】
    // 找到中间位置
    RandomIt mid = begin + len / 2;

    // 4. 【异步启动左半边】
    // 关键点：
    // - 使用 std::async
    // - 策略选 std::launch::async (强制新线程)
    // - 递归调用 parallel_merge_sort
    // - 参数是 begin 和 mid
    // - 这里的返回值类型是 std::future<void>，因为函数返回 void
    if (len > 100000)
    {
        //只有数据量足够大才开线程
        auto left_future = std::async(std::launch::async | std::launch::deferred, [begin, mid]() {
            parallel_merge_sort<RandomIt>(begin, mid);
        });
        parallel_merge_sort(mid,end);
        // - 左半边：正在后台线程排序...
        // - 右半边：已经在当前线程排好了。
        // 必须等待左半边也排好，才能合并。
        left_future.get();
    } else
    {
        parallel_merge_sort(begin, mid);
        parallel_merge_sort(mid,end);
    }

    // 7. 【合并有序序列】
    std::inplace_merge(begin,mid,end);

}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    // 0. 准备工作
    // 工业场景通常是 5000万级别的数据
    const size_t DATA_SIZE = 50000000;
    std::cout << "========================================" << std::endl;
    std::cout << "CPU 核心数: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "测试数据量: " << DATA_SIZE << " 个整数" << std::endl;
    std::cout << "正在生成随机数据..." << std::endl;

    // 生成随机数据
    std::vector<int> data_source(DATA_SIZE);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(1, 1000000000);
    for (auto& num : data_source) {
        num = dis(gen);
    }

    // 复制两份一样的数据，公平竞争
//    std::vector<int> data_seq = data_source;
    std::vector<int> data_par = data_source;

    std::cout << "数据生成完毕，开始测试！" << std::endl;
    std::cout << "========================================" << std::endl;

    // -------------------------------------------------
    // 测试 1: 标准库 std::sort (单线程)
    // -------------------------------------------------
    auto start_seq = std::chrono::high_resolution_clock::now();
    std::sort(data_source.begin(), data_source.end());
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto duration_seq = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq);
    std::cout << "[单线程 std::sort] 耗时: " << duration_seq.count() << " ms" << std::endl;

    // -------------------------------------------------
    // 测试 2: 你的 parallel_merge_sort (多线程)
    // -------------------------------------------------
    auto start_par = std::chrono::high_resolution_clock::now();
    parallel_merge_sort(data_par.begin(), data_par.end());
    auto end_par = std::chrono::high_resolution_clock::now();
    auto duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par);
    std::cout << "[你的多线程 Sort]  耗时: " << duration_par.count() << " ms" << std::endl;

    // -------------------------------------------------
    // 结果对比
    // -------------------------------------------------
    std::cout << "----------------------------------------" << std::endl;
    if (duration_par.count() < duration_seq.count()) {
        double speedup = (double)duration_seq.count() / (double)duration_par.count();
        std::cout << "✅ 成功加速！加速比: " << speedup << "x" << std::endl;
    } else {
        std::cout << "❌ 没有加速 (可能数据量太小或线程开销太大)" << std::endl;
    }

    // -------------------------------------------------
    // 正确性验证
    // -------------------------------------------------
    std::cout << "正在验证正确性..." << std::endl;
    bool is_correct = std::is_sorted(data_par.begin(), data_par.end());
    if (is_correct) {
        std::cout << " 验证通过：数据完全有序！" << std::endl;
    } else {
        std::cout << " 验证失败：数据没有排好序！" << std::endl;
    }

    return 0;
}
