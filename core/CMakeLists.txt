# 指定 CMake 最低版本
cmake_minimum_required(VERSION 3.15)

# 添加静态库，列出所有源文件
add_library(platform_core STATIC
    src/platform/game_registry.cpp
    src/platform/rollback_engine.cpp   # ← 新增
    src/games/fighting_game.cpp
)

# 指定头文件目录
target_include_directories(platform_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置 C++ 标准为 C++17
target_compile_features(platform_core PUBLIC cxx_std_17)