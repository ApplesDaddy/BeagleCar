# credit: https://stackoverflow.com/questions/50760024/cmake-configuration-for-ffmpeg-in-c-project
# credit: Bogdan's CMakeLists.txt
# credit: Microsoft Copilot for debugging errors (last line)
# credit: Chatgpt for helping with cross-compiling errors
    # linux commands to find where libraries where installed, and defining libraries manually

set(FFMPEG_INCLUDE_DIRS "/usr/include/aarch64-linux-gnu")
set(FFMPEG_LIBRARY_DIRS "/usr/lib/aarch64-linux-gnu")

set(FFMPEG_LIBRARIES
    avcodec
    avformat
    avutil
    swscale
)

include_directories(${FFMPEG_INCLUDE_DIRS})

include_directories(vidStreamer/include)
file(GLOB MY_SOURCES "src/*.cpp")

add_library(vidStreamer STATIC ${MY_SOURCES})
target_include_directories(vidStreamer PUBLIC include)
target_link_libraries(vidStreamer PRIVATE ${FFMPEG_LIBRARIES})