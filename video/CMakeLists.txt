add_library(video STATIC v4l2.cpp)
include_directories("${PROJECT_SOURCE_DIR}/include")  # 需要包括上层头文件
target_include_directories(video INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})



# message("工程目录${PROJECT_SOURCE_DIR}")
# message("v4l2目录${CMAKE_CURRENT_SOURCE_DIR}")

