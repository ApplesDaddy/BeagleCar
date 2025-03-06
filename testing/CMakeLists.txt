
# For the webServerTest executable
add_executable(webServerTest src/webServerTest.cpp)
target_link_libraries(webServerTest LINK_PRIVATE webServer)
target_link_libraries(webServerTest PRIVATE crow)

add_custom_command(TARGET webServerTest POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy 
"$<TARGET_FILE:webServerTest>" 
"~/cmpt433/public/webServerTest"
COMMENT "Copying webServerTest to public folder")