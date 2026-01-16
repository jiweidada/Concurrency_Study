//
// Created by Administrator on 2026/1/16.
//

#include "day_1_Safe_Bank_Counter.h"

void SafeBankCounter::Deposit(double amount)
{
    std::lock_guard<std::mutex> lockGuard(mtx);
    balance += amount;
}

State SafeBankCounter::Withdraw(double amount)
{
    std::lock_guard<std::mutex> lockGuard(mtx);
    if (balance >= amount)//检查余额
    {
        balance -= amount;//扣款
        return State::WithdrawTrue;
    } else{
        return State::WithdrawFalse;
    }
}

double SafeBankCounter::GetBalance()
{
    std::lock_guard<std::mutex> lockGuard(mtx);
    double currentBalance = balance;
    return currentBalance;
}