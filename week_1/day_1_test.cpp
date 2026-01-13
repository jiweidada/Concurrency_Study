//
// Created by Administrator on 2026/1/13.
//
#include <iostream>

int main() {
    std::cout << "Hello, C++ Concurrency!\n";
    std::cout << "Compiler check: __cplusplus = " << __cplusplus << std::endl;

    // 检查是否支持C++11
#if __cplusplus >= 201103L
    std::cout << "C++11 or later is supported!\n";
#else
    std::cout << "C++11 is NOT supported. Need newer compiler.\n";
#endif

    return 0;
}