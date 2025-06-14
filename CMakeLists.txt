cmake_minimum_required(VERSION 3.0)
project(litelogcpp LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加可执行文件
add_executable(litelog_samples
    src/Samples.cpp
)

# find_package(Threads REQUIRED)
target_link_libraries(litelog_samples pthread)

# 根据平台设置编译定义
if(WIN32)
    target_compile_definitions(litelog_samples PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
endif()

