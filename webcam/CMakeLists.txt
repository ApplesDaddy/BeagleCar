set(CRASH_DETECT2 ${CMAKE_SOURCE_DIR}/webcam/crash-detection2.py)
set(CRASH_DETECT3 ${CMAKE_SOURCE_DIR}/webcam/crash-detection3.py)
set(DEST_DIR "$ENV{HOME}/cmpt433/public/myApps")

add_custom_target(copy_python_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CRASH_DETECT2} ${DEST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CRASH_DETECT3} ${DEST_DIR}
    COMMENT "Copying crash-detection2.py and crash-detection3.py to myApps"
)
