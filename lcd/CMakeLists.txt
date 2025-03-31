# CMakeList.txt for LCD


include_directories(lcd/lib/Config/)
include_directories(lcd/lib/Fonts/)
include_directories(lcd/lib/GUI/)
include_directories(lcd/lib/LCD/)
file(GLOB MY_SOURCES "lib/Config/*.c" "lib/Fonts/*.c" "lib/GUI/*.c" "lib/LCD/*.c")

add_library(lcd STATIC ${MY_SOURCES})


# Remove extra compiler options that were set in base.
# Ref: https://discourse.cmake.org/t/how-to-disable-pedantic-compiler-option-for-a-specific-library/1575
get_target_property(target_options lcd COMPILE_OPTIONS)
list(REMOVE_ITEM target_options "-Wpedantic")
list(REMOVE_ITEM target_options "-Wextra")
list(REMOVE_ITEM target_options "-Werror")
list(REMOVE_ITEM target_options "-Wall")
set_property(TARGET lcd PROPERTY COMPILE_OPTIONS ${target_options})

# Setup the include paths for where modules that use this library will search.
target_include_directories(lcd PUBLIC lib/Config/)
target_include_directories(lcd PUBLIC lib/Fonts/)
target_include_directories(lcd PUBLIC lib/GUI/)
target_include_directories(lcd PUBLIC lib/LCD/)

# Select lgpio as the library
add_compile_definitions(USE_DEV_LIB)

# Make use of the libraries
target_link_libraries(lcd LINK_PRIVATE lgpio)

