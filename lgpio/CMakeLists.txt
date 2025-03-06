# CMakeList.txt for lgpio

add_library(lgpio STATIC
    lgCtx.c
    lgDbg.c
    lgErr.c
    lgGpio.c
    lgHdl.c
    lgI2C.c
    lgNotify.c
    lgPthAlerts.c
    lgPthTx.c
    lgSerial.c
    lgSPI.c
    lgThread.c
    lgUtil.c
)

# Remove extra compiler options that were set in base.
# Ref: https://discourse.cmake.org/t/how-to-disable-pedantic-compiler-option-for-a-specific-library/1575
get_target_property(target_options lgpio COMPILE_OPTIONS)
list(REMOVE_ITEM target_options "-Wpedantic")
list(REMOVE_ITEM target_options "-Wextra")
list(REMOVE_ITEM target_options "-Werror")
list(REMOVE_ITEM target_options "-Wall")
set_property(TARGET lgpio PROPERTY COMPILE_OPTIONS ${target_options})

# Setup the include paths for where modules that use this library will search.
target_include_directories(lgpio PUBLIC .)
include_directories(lgpio)

add_compile_options(-D_BSD_SOURCE)
