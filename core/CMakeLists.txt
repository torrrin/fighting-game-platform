# 指定 CMake 最低版本
cmake_minimum_required(VERSION 3.15)

# 添加静态库
add_library(platform_core STATIC
    # 源文件暂时为空，后续添加 .cpp 文件时再填入
)

# 指定头文件目录
target_include_directories(platform_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置 C++ 标准为 C++17
target_compile_features(platform_core PUBLIC cxx_std_17)