# C++ Concurrency Study (C++ 并发编程学习实战)

![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg) ![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg) ![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)

本项目记录了我在学习 C++ 高级并发编程（Concurrency）过程中的代码实践、踩坑记录和心得体会。项目从基础的 `std::thread` 开始，逐步深入到互斥锁、条件变量、线程池等高级主题。

## 📂 项目结构

```text
Concurrency_Study/
├── week_1/             # 基础入门
│   ├── day_1_test.cpp  # 线程创建与管理
│   └── ...
├── week_2/             # 进阶与实战
│   ├── day_1_Safe_Bank_Counter.h/cpp  # 实战：线程安全的银行柜台 (RAII锁管理)
│   └── test_safebankcounter.cpp       # 多线程存取款压力测试
├── CMakeLists.txt      # 项目构建配置
└── README.md           # 项目说明
