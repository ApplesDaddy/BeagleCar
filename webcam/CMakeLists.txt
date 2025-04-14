set(CRASH_DETECT2 ${CMAKE_SOURCE_DIR}/webcam/crash-detection2.py)
set(CRASH_DETECT3 ${CMAKE_SOURCE_DIR}/webcam/crash-detection3.py)
set(DEST_DIR "$ENV{HOME}/cmpt433/public/myApps")

add_custom_target(copy_python_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CRASH_DETECT2} ${DEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CRASH_DETECT3} ${DEST_DIR}
    COMMENT "Copying crash-detection2.py and crash-detection3.py to myApps"
)

# For the capture_exec executable
add_executable(capture_exec capture.c)
add_custom_command(TARGET capture_exec POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy
"$<TARGET_FILE:motorTest>"
"~/cmpt433/public/myApps/capture_exec"
COMMENT "Copying capture_exec to public folder")