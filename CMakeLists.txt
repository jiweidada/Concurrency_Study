cmake_minimum_required(VERSION 3.14)
project(Concurrency_Study LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === 新增这一行：强制解决 C4819 中文乱码问题 ===
add_compile_options(
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>"
)

# =============================================================
# 1. 关键路径配置 (直接复用你提供的 sxz_vision 配置)
# =============================================================
# OpenCV 运行库目录 (用于最后拷贝 DLL)
set(OPENCV_BIN_DIR  "D:/opencv4.10/install/x64/vc16/bin")

# CUDA 路径 (解决 OpenCV 报错的核心)
set(CUDAToolkit_ROOT "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.8")
set(CUDA_TOOLKIT_ROOT_DIR "${CUDAToolkit_ROOT}")
set(CUDA_BIN_DIR    "${CUDAToolkit_ROOT}/bin")

# HALCON 路径 (如果你现在的代码没用到 Halcon，可以注释掉下面这几行)
set(HALCON_ROOT "C:/Users/Administrator/AppData/Local/Programs/MVTec/HALCON-24.11-Progress-Steady")
set(HALCON_ARCH "x64-win64")
set(HALCON_LIB_DIR "${HALCON_ROOT}/lib/${HALCON_ARCH}")
set(HALCON_BIN_DIR "${HALCON_ROOT}/bin/${HALCON_ARCH}")

# =============================================================
# 2. 查找依赖包
# =============================================================

# 2.1 查找 CUDA (必须在查找 OpenCV 之前！)
find_package(CUDAToolkit REQUIRED)
# 这一步是为了让 OpenCV 的配置文件能找到 CUDA，从而不报错
include_directories(${CUDAToolkit_INCLUDE_DIRS})

# 2.2 查找 OpenCV
set(OpenCV_DIR "D:/opencv4.10/install/x64/vc16/lib")
find_package(OpenCV CONFIG REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 2.3 查找线程库
find_package(Threads REQUIRED)

# =============================================================
# 3. 如果你需要 HALCON 支持 (可选)
# =============================================================
# 仅当你需要在 main.cpp 中调用 Halcon 时保留此段
include_directories(${HALCON_ROOT}/include ${HALCON_ROOT}/include/halconcpp)

add_library(HALCON::halcon SHARED IMPORTED)
set_target_properties(HALCON::halcon PROPERTIES
        IMPORTED_IMPLIB "${HALCON_LIB_DIR}/halcon.lib"
        IMPORTED_LOCATION "${HALCON_BIN_DIR}/halcon.dll"
)

add_library(HALCON::halconcpp SHARED IMPORTED)
set_target_properties(HALCON::halconcpp PROPERTIES
        IMPORTED_IMPLIB "${HALCON_LIB_DIR}/halconcpp.lib"
        IMPORTED_LOCATION "${HALCON_BIN_DIR}/halconcpp.dll"
)

# =============================================================
# 4. 构建可执行文件
# =============================================================
add_executable(
        Concurrency_Study
#        main.cpp
#        week_1/day_1_test.cpp
        week_1/day_2_test.cpp
)

# 链接库
target_link_libraries(Concurrency_Study PRIVATE
        Threads::Threads     # 基础线程支持
        ${OpenCV_LIBS}       # OpenCV
        CUDA::cudart         # CUDA 运行时 (OpenCV 需要)
        # HALCON::halconcpp  # 如果需要 Halcon，取消注释这一行
)

# =============================================================
# 5. [神器] 自动拷贝 DLL 到运行目录
#    这解决了 "找不到 xxx.dll" 的问题
# =============================================================
set(_runtime_dirs "")
# 将需要拷贝的 DLL 目录加入列表
foreach(dir "${OPENCV_BIN_DIR}" "${CUDA_BIN_DIR}" "${HALCON_BIN_DIR}")
    if(EXISTS "${dir}")
        list(APPEND _runtime_dirs "${dir}")
    endif()
endforeach()

# 编译完成后，自动把 DLL 拷过去
foreach(dir IN LISTS _runtime_dirs)
    add_custom_command(TARGET Concurrency_Study POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${dir}" "$<TARGET_FILE_DIR:Concurrency_Study>"
            COMMENT "Copying runtime DLLs from: ${dir}"
    )
endforeach()