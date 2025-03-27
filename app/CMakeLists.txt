# Build udp testing app

include_directories(include)
add_executable(udp src/udp.c src/receiver.c src/sender.c)

# Make use of the HAL library
target_link_libraries(udp LINK_PRIVATE hal)
target_link_libraries(udp LINK_PRIVATE util)

# Support GPIO
find_library(udp gpiod)
target_link_libraries(udp LINK_PRIVATE gpiod)

# Support ncurses (only needed to test udp sending with terminal)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(udp PRIVATE ${CURSES_LIBRARIES})

# PTHREAD support
find_package(Threads REQUIRED)
target_link_libraries(udp LINK_PRIVATE Threads::Threads)
target_link_libraries(udp PRIVATE m)

# Copy executables to final location if compiling for target
if(CMAKE_C_COMPILER STREQUAL "aarch64-linux-gnu-gcc")
    add_custom_command(TARGET udp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:udp>"
        "$ENV{HOME}/cmpt433/public/myApps/"
    COMMENT "Copying UDP ARM executable to public NFS directory")
endif()
