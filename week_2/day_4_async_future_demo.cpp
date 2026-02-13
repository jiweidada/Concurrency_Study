#include <iostream>
#include <future>
#include <thread>
#include <chrono>

// 仅在 Windows 下包含 windows.h 并设置控制台编码
#ifdef _WIN32
#include <windows.h>
#endif

// 模拟一个耗时的图像处理任务
//std::vector<double> process_image(const std::string& image_name) {
//    std::cout << "[后台] 开始处理图像: " << image_name << " (线程ID: " << std::this_thread::get_id() << ")\n";
//    std::this_thread::sleep_for(std::chrono::seconds(2)); // 模拟耗时
//    return std::vector<double>{1.0, 2.0, 3.0}; // 返回检测到的缺陷数量
//}
//
//void doing_other_things() {
//    std::cout << "[主线程] 我正在做其他事情... (线程ID: " << std::this_thread::get_id() << ")\n";
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//}

//void complex_calculation(std::promise<int> prom) {
//    std::cout << "[工作线程] 正在计算...\n";
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//    // 假设发生了一个错误，我们可以设置异常，也可以设置值
//    bool success = true;
//    if (success) {
//        // 承诺兑现：设置值
//        prom.set_value(100);
//    } else {
//        // 也可以传递异常给主线程
//        prom.set_value(-1);
////      prom.set_exception(std::make_exception_ptr(std::runtime_error("计算失败")));
//    }
//    std::cout << "[工作线程] 结果已设置，任务结束。\n";
//}

int calculate_sqrt(int x) {
    return std::sqrt(x);
}

int main() {

    // 仅在 Windows 下执行控制台编码设置
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

      ///   std::async：最简单的异步启动
    {
    //    std::cout << "[主线程] 准备启动任务...\n";
    //    // 1. 启动异步任务
    //    // std::launch::async 强制创建一个新线程来执行
    //    std::future<std::vector<double>> defect_counts_future = std::async(std::launch::async, process_image, "defect_001.bmp");
    //    std::cout << "[主线程] 任务已提交，我继续做别的事...\n";
    //    doing_other_things();
    //    std::cout << "[主线程] 别的事做完了，准备获取结果...\n";
    //    // 2. 获取结果
    //    // result_future.get() 会阻塞，直到后台任务完成
    //    std::vector<double> defect_count;
    //    defect_count = defect_counts_future.get();
    //    for (auto i : defect_count)
    //    {
    //        std::cout << i << " ";
    //    }
    //    std::cout << std::endl;
    //    std::cout << "[主线程] 拿到结果: 缺陷数量 = " << defect_count.size() << "\n";
    }

    ///     std::promise：手动传递结果
    {
//        // 1. 创建一个 promise (发送口)
//        std::promise<int> my_promise;
//        // 2. 获取关联的 future (接收口)
//        std::future<int> my_future = my_promise.get_future();
//        // 3. 把 promise 移动给新线程 (promise 不能复制，只能移动)
//        std::thread thread(complex_calculation, std::move(my_promise));
//
//        std::cout << "[主线程] 等待结果...\n";
//
//        // 4. 阻塞等待结果
//        int val = my_future.get();
//
//        std::cout << "[主线程] 收到结果: " << val << "\n";
//        thread.join();

    }

    ///    std::packaged_task：包装任务
    {
        // 1. 打包任务：将函数 calculate_sqrt 包装起来
        // 模板参数 <int(int)> 表示返回值 int，参数 int
        std::packaged_task<int(int)> task(calculate_sqrt);
        // 2. 预先拿到 future
        std::future<int> ret = task.get_future();
        // 3. 执行任务
        // 方式 A: 直接在当前线程调用 (不推荐，没并发意义)
        // task(16);
        // 方式 B: 丢给一个新线程去跑
        std::thread t(std::move(task), 25);
        // 4. 获取结果
        std::cout << "Result: " << ret.get() << std::endl; // 输出 5

        t.join();
    }

    return 0;
}