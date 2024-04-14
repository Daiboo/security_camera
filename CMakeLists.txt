cmake_minimum_required(VERSION 3.16)

project(security_camera VERSION 0.1.0 LANGUAGES C CXX)


# 二进制文件输出路径为./
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(SRC_LIST 
    "video/v4l2.cpp"
    "test/video_test.cpp"
)

set(INC_LIST
    "include"
    "include/video"
)


include_directories(${INC_LIST})
add_executable(security_camera ${SRC_LIST})

# message("头文件目录${INC_LIST}")

