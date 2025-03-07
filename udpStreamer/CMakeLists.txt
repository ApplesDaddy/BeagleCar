# credit: https://stackoverflow.com/questions/50760024/cmake-configuration-for-ffmpeg-in-c-project
# credit: Bogdan's CMakeLists.txt
# credit: Microsoft Copilot for debugging errors (last line)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
)
include_directories(${FFMPEG_INCLUDE_DIRS})
include_directories(udpStreamer/include)
file(GLOB MY_SOURCES "src/*.cpp")

add_library(udpStreamer STATIC ${MY_SOURCES})
target_include_directories(udpStreamer PUBLIC include)
target_link_libraries(udpStreamer PRIVATE PkgConfig::FFMPEG)

