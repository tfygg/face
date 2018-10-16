cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)
project(face_morphmodel)

# add snow
set(SNOW_MODULE_OPENGL on CACHE BOOL "enable opengl" FORCE)  # set module on
add_subdirectory(../ext/snow/snow/ ext/snow)
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(${SNOW_LIBRARIES_DIRS})
add_definitions(${SNOW_DEFINITIONS})

# exec
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/morphable/basel.cpp
    ${PROJECT_SOURCE_DIR}/visualization/window.cpp
)
target_link_libraries(${PROJECT_NAME} snow)
