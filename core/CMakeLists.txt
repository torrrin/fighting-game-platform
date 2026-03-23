# 声明这是一个 CMake 项目
cmake_minimum_required(VERSION 3.15)

# 添加一个静态库，名字叫 platform_core
# STATIC 表示编译成静态库（.a 文件）
add_library(platform_core STATIC
    # 这里暂时为空，后续添加 .cpp 文件时再填入
)

# 指定头文件目录，让其他代码能找到我们的头文件
target_include_directories(platform_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)