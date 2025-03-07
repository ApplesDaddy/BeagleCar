
# For the webServerTest executable
add_executable(webServerTest src/webServerTest.cpp)
target_link_libraries(webServerTest LINK_PRIVATE webServer)
target_link_libraries(webServerTest PRIVATE crow)

add_custom_command(TARGET webServerTest POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy
"$<TARGET_FILE:webServerTest>"
"~/cmpt433/public/webServerTest"
COMMENT "Copying webServerTest to public folder")


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

