cmake_minimum_required(VERSION 3.4.1)

#设置第三方动态库的头文件路径
set(lib_include_DIR ${CMAKE_SOURCE_DIR}/src/main/cpp/include)

#设置第三方动态库的路径
set(lib_src_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})

#导入第三方库的头文件目录
#include_directories(${lib_include_DIR})
include_directories(
        ${CMAKE_SOURCE_DIR}/src/main/cpp/include
)

#添加第三方动态库依赖
add_library(avcodec-57_lib SHARED IMPORTED)
set_target_properties(avcodec-57_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavcodec-57.so)

#添加第三方动态库依赖
add_library(avformat-57_lib SHARED IMPORTED)
set_target_properties(avformat-57_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavformat-57.so)

#添加第三方动态库依赖
add_library(avutil-55_lib SHARED IMPORTED)
set_target_properties(avutil-55_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavutil-55.so)

#添加第三方动态库依赖
add_library(swresample-2_lib SHARED IMPORTED)
set_target_properties(swresample-2_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswresample-2.so)

#添加第三方动态库依赖
add_library(swscale-4_lib SHARED IMPORTED)
set_target_properties(swscale-4_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswscale-4.so)


# 构建本地动态库
add_library(player-lib SHARED
        ${CMAKE_SOURCE_DIR}/src/main/cpp/videoeditor/videoEditor.cpp
        )

# 从系统库中查找依赖库
find_library( # Sets the name of the path variable.
        # 设置依赖库的名字，下面链接库的时候会用到
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.

        # 查找log依赖库
        # {sdk-path}/ndk-bundle/sysroot/usr/include/android/log.h
        log)

# Include libraries needed for VideoPlayer lib
target_link_libraries(player-lib
        log
        android
        OpenSLES
        avcodec-57_lib
        avformat-57_lib
        avutil-55_lib
        swresample-2_lib
        swscale-4_lib
        ${log-lib})
