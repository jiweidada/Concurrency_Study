//
// Created by Administrator on 2026/1/16.
//

#ifndef CONCURRENCY_STUDY_DAY_1_SAFE_BANK_COUNTER_H
#define CONCURRENCY_STUDY_DAY_1_SAFE_BANK_COUNTER_H

#include <mutex>

enum class State
{
    WithdrawTrue = 0,
    WithdrawFalse = 1
};

class SafeBankCounter {
public:
    SafeBankCounter():balance(0){};
    void Deposit(double a);//存款操作
    State Withdraw(double m);// 取款操作
    double GetBalance();//查询余额
private:
    double balance;
    std::mutex mtx;
};


#endif //CONCURRENCY_STUDY_DAY_1_SAFE_BANK_COUNTER_H
