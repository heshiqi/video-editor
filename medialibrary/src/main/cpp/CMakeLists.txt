cmake_minimum_required(VERSION 3.4.1)


include_directories( include )


#添加第三方动态库依赖
add_library(avcodec-57_lib SHARED IMPORTED)
set_target_properties(avcodec-57_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavcodec-.so)

#添加第三方动态库依赖
add_library(avfilter-6_lib SHARED IMPORTED)
set_target_properties(avfilter-6_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavfilter-.so)

#添加第三方动态库依赖
add_library(avformat-57_lib SHARED IMPORTED)
set_target_properties(avformat-57_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavformat-.so)

#添加第三方动态库依赖
add_library(avutil-55_lib SHARED IMPORTED)
set_target_properties(avutil-55_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libavutil-.so)

#添加第三方动态库依赖
add_library(swresample-2_lib SHARED IMPORTED)
set_target_properties(swresample-2_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswresample-.so)

#添加第三方动态库依赖
add_library(swscale-4_lib SHARED IMPORTED)
set_target_properties(swscale-4_lib PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libswscale-.so)

##添加第三方静态库依赖
add_library(x264 STATIC IMPORTED)
set_target_properties(x264 PROPERTIES IMPORTED_LOCATION
        ${lib_src_DIR}/libx264-.so)

