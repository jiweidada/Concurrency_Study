/*通过写这段代码，你掌握了以下 4 个多线程编程的关键技术：
 *
引用传递的奥义 (std::ref)：
你理解了 std::thread 默认是“拷贝”参数。
你正确使用了 std::ref(it)，让子线程直接修改 main 函数里的 CalculationTask 结构体，从而带回了计算结果。这是数据回传最直接的方式。

智能的资源管理 (emplace_back & vector)：
使用了 emplace_back 避免了创建临时的线程对象，性能更高，代码更简洁。
利用 vector 管理一组线程，而不是手写一个个 t1, t2, t3，这让你的程序能处理任意数量的任务。

硬件感知 (hardware_concurrency)
程序不再是“瞎跑”，而是先问 CPU：“你有几个核？”。
根据核心数来决定一次跑多少个线程，防止创建几千个线程把系统卡死。

分批同步调度策略 (Batch Synchronization)：
你实现了一个**“大巴车模式”**的调度：
上客：把任务一个个塞进 vector。
满员发车：一旦 vec_thread.size() >= num_cores，马上停下来等待这一车人跑完 (join)。
清空回场：clear() 清空座位，准备接下一批。
扫尾：最后再检查一下有没有没坐满的“末班车”，有的话也要等它们跑完
 */

#include <thread>
#include <iostream>
#include <vector>
#include <cmath> // 引入这个头文件以支持 std::pow
#include <mutex>

// === 模块 A：数据结构 ===
enum class TaskType {
    Addition = 0,
    Multiplication = 1,
    Power = 2
};

// 使用模板让任务结构体更通用（支持 int, float 等）
template <class T>
struct CalculationTask {
    TaskType tasktype;
    T a;
    T b;
    T result;
    // 构造函数初始化，result 默认为 0
    CalculationTask(TaskType t, T x, T y)
            : tasktype(t), a(x), b(y), result(0) {}
};

// === 模块 B：工作线程函数 ===
// 注意：这里必须接收引用 (&)，否则修改的是副本，主线程拿不到结果
void calculate(CalculationTask<int>& calculationTask)
{
    // 打印当前线程 ID，证明是并行在跑
    std::cout << "Thread " << std::this_thread::get_id()
              << " started calculation\n";

    // 模拟耗时操作 (500ms)，让并发的效果更明显
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 根据任务类型执行计算
    switch (calculationTask.tasktype) {
        case TaskType::Addition :
            calculationTask.result = calculationTask.a + calculationTask.b;
            break;
        case TaskType::Multiplication :
            calculationTask.result = calculationTask.a * calculationTask.b;
            break;
        case TaskType::Power :
            // 强转类型，因为 pow 返回的是 double
            calculationTask.result = static_cast<int>(std::pow(calculationTask.a, calculationTask.b));
            break;
    }
    // 这里的打印其实有竞争风险，但在学习阶段为了看清流程可以保留
    // 生产环境建议加锁 std::lock_guard
    std::mutex mutex;
    {
        std::lock_guard<std::mutex> lockGuard(mutex);
        std::cout << "Thread " << std::this_thread::get_id()
                   << " finished calculation\n";
    }

}

// === 模块 C：主程序 ===
int main() {
    std::cout << "=== Parallel Calculator ===\n\n";

    // 1. 准备数据：12 个任务
    std::vector<CalculationTask<int>> tasks = {
            {TaskType::Addition, 10, 20},
            {TaskType::Multiplication, 5, 6},
            {TaskType::Power, 2, 8},
            {TaskType::Addition, 100, 200},
            {TaskType::Multiplication, 12, 13},
            {TaskType::Power, 3, 4},
            {TaskType::Addition, 10, 20},
            {TaskType::Multiplication, 5, 6},
            {TaskType::Power, 2, 8},
            {TaskType::Addition, 100, 200},
            {TaskType::Multiplication, 12, 13},
            {TaskType::Power, 3, 4}
    };

    // 2. 询问硬件能力
    unsigned int num_cores = std::thread::hardware_concurrency();
    // 兜底策略：如果获取失败（返回0），默认给2个核心，防止除以0或逻辑错误
    if (num_cores == 0) num_cores = 2;
    std::cout << "CPU cores available: " << num_cores << "\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    // 3. 核心调度逻辑（分批处理）
    std::vector<std::thread> vec_thread;

    // 遍历每一个任务
    for (auto &task : tasks) // 建议改名叫 task 增加可读性
    {
        // 创建线程并开始执行
        // 重点：使用 std::ref() 包装，实现引用传递
        vec_thread.emplace_back(calculate, std::ref(task));

        // 流控判断：如果当前正在跑的线程数达到了 CPU 核心数
        if (vec_thread.size() >= num_cores)
        {
            // 等待这一批线程全部跑完（同步点）
            for (auto &thread : vec_thread) {
                if(thread.joinable()) thread.join();
            }
            // 清空容器，释放线程对象，为下一批腾位置
            vec_thread.clear();
            std::cout << "--- Batch finished (Bus arrived) ---\n";
        }
    }

    // 4. 处理尾部任务
    // 比如：12个任务，CPU是8核。
    // 第一批跑了8个，循环里 join 了。
    // 剩下4个任务在循环里被 create 了，但没触发 if 判断，所以还没 join。
    // 这里必须把最后这4个也回收掉。
    for (auto &thread : vec_thread) {
        if(thread.joinable()) thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time
    );

    // 5. 验证结果
    std::cout << "\n=== Results ===\n";
    int task_num = 1;
    for (const auto& task : tasks) {
        std::cout << "Task " << task_num++ << ": Result = " << task.result << "\n";
    }

    std::cout << "\nTotal execution time: " << duration.count()
              << " milliseconds\n";

    return 0;
}