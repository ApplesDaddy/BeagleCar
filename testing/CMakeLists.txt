
# For the webServerTest executable
add_executable(webServerTest src/webServerTest.cpp)
target_link_libraries(webServerTest LINK_PRIVATE webServer)
target_link_libraries(webServerTest PRIVATE crow)

add_custom_command(TARGET webServerTest POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy 
"$<TARGET_FILE:webServerTest>" 
"~/cmpt433/public/webServerTest"
COMMENT "Copying webServerTest to public folder")

# For the udpStreamer executable
add_executable(udpStreamerTest src/udpStreamerTest.cpp)
target_link_libraries(udpStreamerTest LINK_PRIVATE udpStreamer)

add_custom_command(TARGET udpStreamerTest POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy 
"$<TARGET_FILE:udpStreamerTest>" 
"~/cmpt433/public/udpStreamerTest"
COMMENT "Copying udpStreamerTest to public folder")