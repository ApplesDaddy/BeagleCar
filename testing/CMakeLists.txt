
# For the webServerTest executable
add_executable(webServerTest src/webServerTest.cpp)
target_link_libraries(webServerTest LINK_PRIVATE webServer)
target_link_libraries(webServerTest PRIVATE crow)

add_custom_command(TARGET webServerTest POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy 
"$<TARGET_FILE:webServerTest>" 
"~/cmpt433/public/webServerTest"
COMMENT "Copying webServerTest to public folder")

## For testing, move the templates, static and video folder to where the executable is
add_custom_command(TARGET webServerTest POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
"${CMAKE_SOURCE_DIR}/webServer/src/templates"
"${CMAKE_BINARY_DIR}/testing/templates"
COMMENT "Copying templates to testing build folder")

add_custom_command(TARGET webServerTest POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
"${CMAKE_SOURCE_DIR}/webServer/src/static"
"${CMAKE_BINARY_DIR}/testing/static"
COMMENT "Copying static to testing build folder")

add_custom_command(TARGET webServerTest POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
"${CMAKE_SOURCE_DIR}/webServer/video"
"${CMAKE_BINARY_DIR}/testing/video"
COMMENT "Copying video to testing build folder")