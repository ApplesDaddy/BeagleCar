include_directories(webServer/include)
file(GLOB MY_SOURCES "src/*.cpp")

add_library(webServer STATIC ${MY_SOURCES})
target_include_directories(webServer PUBLIC include)