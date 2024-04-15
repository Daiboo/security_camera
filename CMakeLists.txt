cmake_minimum_required(VERSION 3.16)

project(security_camera VERSION 0.1.0 LANGUAGES C CXX)

# specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 二进制文件输出路径为./
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# 添加子目录
add_subdirectory(video)

# 添加可执行文件
add_executable(security_camera "test/video_test.cpp")


# 目标链接库
target_link_libraries(security_camera PUBLIC video)

# 添加头文件
# include_directories("include")
target_include_directories(security_camera PUBLIC "include")


# ****************************DEBUG********************************
# message("头文件目录${INC_LIST}")
# 打印二进制目录
# message("二进制目录${PROJECT_BINARY_DIR}") # /home/daiboo/codes/0_linux_project/0_security_camera/build
# 打印工程目录
message("工程目录${PROJECT_SOURCE_DIR}") # /home/daiboo/codes/0_linux_project/0_security_camera

