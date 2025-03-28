# For the webServerTest executable
add_executable(motorTest src/motorTest.c)
target_link_libraries(motorTest LINK_PRIVATE hal)
add_custom_command(TARGET motorTest POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy
"$<TARGET_FILE:motorTest>"
"~/cmpt433/public/motorTest"
COMMENT "Copying motorTest to public folder")

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


# For the lcd_video_test executable
file(GLOB MY_SOURCES "src/lcdVideoTest.cpp")
add_executable(lcd_video_test ${MY_SOURCES})

# Make use of the libraries
target_link_libraries(lcd_video_test LINK_PRIVATE hal)
target_link_libraries(lcd_video_test LINK_PRIVATE lcd)
target_link_libraries(lcd_video_test LINK_PRIVATE lgpio)

add_custom_command(TARGET lcd_video_test POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy
     "$<TARGET_FILE:lcd_video_test>"
     "~/cmpt433/public/myApps/lcd_video_test"
  COMMENT "Copying lcd_video_test executable to public NFS directory")

