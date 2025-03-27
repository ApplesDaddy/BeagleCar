# CMakeList.txt for HAL
#   Build a library (`hal`) which exposes the header files as "hal/*.h"
#   Use header as: #include "hal/button.h"

include_directories(hal/include)
file(GLOB MY_SOURCES "src/*.c")

add_library(hal STATIC ${MY_SOURCES})

target_include_directories(hal PUBLIC include)
target_link_libraries(hal LINK_PRIVATE lcd)
target_link_libraries(hal LINK_PRIVATE util)


# (host) sudo apt-get install libutil-dev
# (host) sudo apt-get install libavcodec-dev
# (host) sudo apt-get install libavformat-dev
# (host) sudo apt-get install libswscale-dev
# (host) sudo apt-get install ffmpeg
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVUTIL_LIBRARY avutil)
find_library(AVFORMAT_LIBRARY avformat)
find_library(SWSCALE_LIBRARY swscale)
target_link_libraries(hal LINK_PRIVATE ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})
