# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(beagle_car ${MY_SOURCES})

# Make use of the libraries
target_link_libraries(beagle_car LINK_PRIVATE hal)
target_link_libraries(beagle_car LINK_PRIVATE lcd)
target_link_libraries(beagle_car LINK_PRIVATE lgpio)

add_custom_command(TARGET beagle_car POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy
     "$<TARGET_FILE:beagle_car>"
     "~/cmpt433/public/myApps/beagle_car"
  COMMENT "Copying ARM executable to public NFS directory")

