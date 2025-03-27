
# For the webServerTest executable
# add_executable(webServerTest src/webServerTest.cpp)
# target_link_libraries(webServerTest LINK_PRIVATE webServer)
# target_link_libraries(webServerTest PRIVATE crow)

# add_custom_command(TARGET webServerTest POST_BUILD 
# COMMAND "${CMAKE_COMMAND}" -E copy 
# "$<TARGET_FILE:webServerTest>" 
# "~/cmpt433/public/webServerTest"
# COMMENT "Copying webServerTest to public folder")

# For the vidStreamer executable
add_executable(vidStreamerTest src/vidStreamerTest.cpp)
target_link_libraries(vidStreamerTest LINK_PRIVATE vidStreamer)

add_custom_command(TARGET vidStreamerTest POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy 
"$<TARGET_FILE:vidStreamerTest>" 
"~/cmpt433/public/vidStreamerTest"
COMMENT "Copying vidStreamerTest to public folder")