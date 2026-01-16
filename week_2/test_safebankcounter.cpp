//
// Created by Administrator on 2026/1/16.
//
#include <vector>
#include <iostream>
#include <windows.h>
#include "day_1_Safe_Bank_Counter.h"

int main()
{
    SetConsoleOutputCP(65001);

    SafeBankCounter safebankcounter;
    double money = 100.0;
    // 创建多个线程
    std::vector<std::thread> vec_threads;
    for (int i = 0; i < 10; ++i) {
        vec_threads.emplace_back(&SafeBankCounter::Deposit,&safebankcounter,money);
    }
    std::vector<State> results(10);
    for (int i = 0; i < 10; ++i) {
//        vec_threads.push_back(
//                // 必须显式调用 std::thread 的构造函数
//                std::thread([&safebankcounter, &results, i, money]() {
//                    results[i] = safebankcounter.Withdraw(money);
//                })
//        );
        vec_threads.emplace_back(
                [&safebankcounter, &results, i, money](){
                    results[i] = safebankcounter.Withdraw(money);
                }
                );
    }

    for (auto &thread:vec_threads) {
        if (thread.joinable())   thread.join();
    }

    // 打印结果，看看谁失败了
    int success_count = 0;
    int fail_count = 0;

    std::cout << "=== 取款详情 ===" << std::endl;
    for (int i = 0; i < 10; ++i) {
        if (results[i] == State::WithdrawTrue) {
            // std::cout << "线程 " << i << ": 成功" << std::endl;
            success_count++;
        } else {
            // std::cout << "线程 " << i << ": 失败 (余额不足)" << std::endl;
            fail_count++;
        }
    }

    std::cout << "成功次数: " << success_count << std::endl;
    std::cout << "失败次数: " << fail_count << std::endl;
    std::cout << "最终余额: " << safebankcounter.GetBalance() << std::endl;
    return 0;
}


