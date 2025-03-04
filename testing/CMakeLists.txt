#include_directories(include)
#file(GLOB MY_SOURCES "src/*.c")
add_executable(webServerTest src/webServerTest.c)

# Make use of the libraries
target_link_libraries(webServerTest LINK_PRIVATE webServer)