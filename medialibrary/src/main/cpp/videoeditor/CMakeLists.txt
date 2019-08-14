
include_directories(
       ../common
)

# 构建本地动态库
add_library(player-lib SHARED
        videoEditor.cpp
        video_cut.cpp
        ../common/ffmpeg_utils.c
        video2image.c
        ../factory/entity_factory.c
        ../testplayer/h_media_player.cpp
        ../testplayer/h_java_call.cpp
        ../testplayer/h_ffmpeg.cpp
        ../testplayer/h_base_player.cpp
        ../testplayer/h_queue.cpp
        ../testplayer/h_play_status.cpp
        ../testplayer/h_audio.cpp
        ../testplayer/h_video.cpp
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
        avfilter-6_lib
        x264
        ${log-lib})
