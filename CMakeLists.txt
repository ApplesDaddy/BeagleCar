# CMake Build Configuration for root of project
cmake_minimum_required(VERSION 3.18)
project(Beagle_Car
    VERSION 1.0
    DESCRIPTION "CMPT433 - Project Beagle Car"
    LANGUAGES C CXX
)

# Compiler options (inherited by sub-folders)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu) # so ld uses the correct version
add_compile_options(-Wall -Wpedantic -Wextra -g)
# add_compile_options(-Werror)
#crow_all.h gives a error due to a condition
#always being met so turning this off for the moment
add_compile_options(-fdiagnostics-color)

# Enable address sanitizer
# (Comment this out to make your code faster)
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# Enable PThread library for linking
add_compile_options(-pthread)
add_link_options(-pthread)


find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

# What folders to build
add_subdirectory(header_files)
add_subdirectory(webServer)
add_subdirectory(testing)

add_subdirectory(hal)
add_subdirectory(lcd)
add_subdirectory(app)
add_subdirectory(lgpio)