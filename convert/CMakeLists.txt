
include(${PROJECT_SOURCE_DIR}/library.cmake)
add_library(convert STATIC convert_manager.cpp mjpeg2rgb.cpp)
target_link_libraries(convert PUBLIC ${JPEG_LIBRARY} video)
include_directories("${PROJECT_SOURCE_DIR}/include")  # 需要包括上层头文件
target_include_directories(convert INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC "${LIBJPEG_DIR}/include")


