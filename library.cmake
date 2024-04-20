# 外部库文件
find_package(Qt5 COMPONENTS Widgets REQUIRED)
set(LIBJPEG_DIR "/home/daiboo/toolchain/libjpeg-turbo-2.1.5.1/build/install_x86_64")
find_library(JPEG_LIBRARY NAMES jpeg PATHS ${LIBJPEG_DIR})