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
add_executable(security_camera "main.c")


# 找到Qt库
find_package(Qt5 COMPONENTS Widgets REQUIRED)
set(LIBJPEG_DIR "/home/daiboo/toolchain/libjpeg-turbo-2.1.5.1/build/install_x86_64")
find_library(JPEG_LIBRARY NAMES jpeg PATHS ${LIBJPEG_DIR})

# 目标链接库
target_link_libraries(security_camera PUBLIC video)

# 添加头文件
# include_directories("include")
target_include_directories(security_camera PUBLIC "include")

# ----------------------------TEST-------------------------------
add_executable(video_test "test/video_test.cpp")
# 设置video_test输出目录
set_target_properties(video_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
target_link_libraries(video_test PUBLIC video ${JPEG_LIBRARY})
target_include_directories(video_test PUBLIC "include" "${LIBJPEG_DIR}/include")

# 创建一个可执行文件
add_executable(window_test "test/win_test.cpp")
set_target_properties(window_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
target_link_libraries(window_test PUBLIC Qt5::Widgets video ${JPEG_LIBRARY})
target_include_directories(window_test PUBLIC "include" "${LIBJPEG_DIR}/include")

add_executable(qtlabel "test/qtlabel.cpp")
set_target_properties(qtlabel PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
target_link_libraries(qtlabel PUBLIC Qt5::Widgets)
target_include_directories(qtlabel PUBLIC "include")

# ****************************DEBUG********************************
# message("头文件目录${INC_LIST}")
# 打印二进制目录
# message("二进制目录${PROJECT_BINARY_DIR}") # /home/daiboo/codes/0_linux_project/0_security_camera/build
# 打印工程目录
# message("工程目录${PROJECT_SOURCE_DIR}") # /home/daiboo/codes/0_linux_project/0_security_camera
# message("库问年间: ${JPEG_LIBRARY}")


