include_directories(webServer/include)
file(GLOB MY_SOURCES "src/*.cpp")

add_library(webServer STATIC ${MY_SOURCES})

target_include_directories(webServer PUBLIC include)
target_link_libraries(webServer PRIVATE crow)

# Copies the static and template directories to public to make them accessible on the beagle
add_custom_command(TARGET webServer POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy_directory 
     "${CMAKE_CURRENT_SOURCE_DIR}/src/static"
     "~/cmpt433/public/myApps/static" 
  COMMENT "Copying static directory to public folder")
  add_custom_command(TARGET webServer POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy_directory 
     "${CMAKE_CURRENT_SOURCE_DIR}/src/templates"
     "~/cmpt433/public/myApps/templates" 
  COMMENT "Copying templates directory to public folder")