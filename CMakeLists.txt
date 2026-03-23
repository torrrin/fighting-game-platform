# 根目录 CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(FightingGamePlatform VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_TOOLS "Build development tools" ON)

# 添加核心库子目录
add_subdirectory(core)

if(BUILD_TESTS)
    enable_testing()
    # 测试目录稍后添加
    # add_subdirectory(core/tests)
endif()

if(BUILD_TOOLS)
    # 工具目录稍后添加
    # add_subdirectory(tools)
endif()

message(STATUS "Build configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build tools: ${BUILD_TOOLS}")

# 如果是 Emscripten 构建，添加 web 子目录
if(EMSCRIPTEN)
    add_subdirectory(web)
endif()