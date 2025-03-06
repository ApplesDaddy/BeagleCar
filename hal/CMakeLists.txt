# CMakeList.txt for HAL
#   Build a library (`hal`) which exposes the header files as "hal/*.h"
#   Use header as: #include "hal/button.h"

include_directories(hal/include)
file(GLOB MY_SOURCES "src/*.c")

add_library(hal STATIC ${MY_SOURCES})

target_include_directories(hal PUBLIC include)
target_link_libraries(hal LINK_PRIVATE lcd)


# (host) sudo apt-get install ffmpeg:arm64
# (target) sudo apt-get install ffmpeg
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVUTIL_LIBRARY avutil)
target_link_libraries(hal LINK_PRIVATE ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY})