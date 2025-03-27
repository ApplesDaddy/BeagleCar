# CMakeList.txt for HAL
#   Build a library (`util`) which exposes the header files as "util/*.h"
#   Use header as: #include "util/button.h"

include_directories(util/include)
file(GLOB MY_SOURCES "src/*.c")

add_library(util STATIC ${MY_SOURCES})

target_include_directories(util PUBLIC include)
