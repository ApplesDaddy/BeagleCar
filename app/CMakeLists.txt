# Build udp testing app

include_directories(include)
file(GLOB MY_SOURCES "src/*.c" "src/*.cpp")

add_executable(controller src/controller.cpp src/lcd_stream_recv.cpp src/sender.c)
add_executable(udp src/udp.c src/receiver.c src/sender.c)

# Make use of the HAL library
target_link_libraries(controller LINK_PRIVATE hal)
target_link_libraries(controller LINK_PRIVATE util)


target_link_libraries(udp LINK_PRIVATE hal)
target_link_libraries(udp LINK_PRIVATE util)

target_link_libraries(controller LINK_PRIVATE lcd)
target_link_libraries(controller LINK_PRIVATE lgpio)

# Support GPIO
find_library(udp gpiod)
target_link_libraries(udp LINK_PRIVATE gpiod)

target_link_libraries(controller LINK_PRIVATE gpiod)

# Support ncurses (only needed to test udp sending with terminal)
set(CURSES_INCLUDE_PATH /usr/include)
set(CURSES_LIBRARY /usr/lib/aarch64-linux-gnu/libncurses.so)
find_package(Curses REQUIRED)
target_link_libraries(controller PRIVATE ${CURSES_LIBRARY})

target_link_libraries(udp PRIVATE ${CURSES_LIBRARY})
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(udp PRIVATE ${CURSES_LIBRARIES})

# PTHREAD support
find_package(Threads REQUIRED)
target_link_libraries(controller LINK_PRIVATE Threads::Threads)
target_link_libraries(controller PRIVATE m)

target_link_libraries(udp LINK_PRIVATE Threads::Threads)
target_link_libraries(udp PRIVATE m)

# Webserver
target_link_libraries(controller LINK_PRIVATE webServer)
target_link_libraries(controller PRIVATE crow)

# Copy executables to final location if compiling for target
if(CMAKE_C_COMPILER STREQUAL "aarch64-linux-gnu-gcc")
    add_custom_command(TARGET controller POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:controller>"
        "$ENV{HOME}/cmpt433/public/myApps/"
    COMMENT "Copying Controller ARM executable to public NFS directory")
endif()

if(CMAKE_C_COMPILER STREQUAL "aarch64-linux-gnu-gcc")
    add_custom_command(TARGET udp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:udp>"
        "$ENV{HOME}/cmpt433/public/myApps/"
    COMMENT "Copying udp ARM executable to public NFS directory")

    add_custom_command(TARGET udp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "${CMAKE_SOURCE_DIR}/wifi_config_files/wifi_ap.sh"
        "$ENV{HOME}/cmpt433/public/myApps/"
    COMMENT "Copying wifi ap script to public NFS directory")

    add_custom_command(TARGET udp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "${CMAKE_SOURCE_DIR}/wifi_config_files/wifi_client.sh"
        "$ENV{HOME}/cmpt433/public/myApps/"
    COMMENT "Copying wifi client script to public NFS directory")
endif()

add_custom_command(TARGET controller POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_SOURCE_DIR}/webServer/src/templates"
    "$ENV{HOME}/cmpt433/public/myApps/templates"
COMMENT "Copying templates to build folder")

add_custom_command(TARGET controller POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_SOURCE_DIR}/webServer/src/static"
    "$ENV{HOME}/cmpt433/public/myApps/static"
COMMENT "Copying static to build folder")
