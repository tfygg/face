cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 11)
project(snow_face)

# add snow
set(SNOW_MODULE_OPENGL on CACHE BOOL "enable opengl" FORCE)  # set module on
set(SNOW_MODULE_FFMPEG on CACHE BOOL "enable ffmpeg" FORCE)  # set module on
add_subdirectory(../ext/snow/snow/ ext/snow)
include_directories(${SNOW_INCLUDE_DIRS})
link_directories(${SNOW_LIBRARIES_DIRS})
add_definitions(${SNOW_DEFINITIONS})

set(FACE_MORPH "face_morphmodel")
set(FACE_ALIGN "face_alignment")
set(FACE_MORPH_DIR  "${PROJECT_SOURCE_DIR}/morphmodel")
set(FACE_ALIGN_DIR  "${PROJECT_SOURCE_DIR}/alignment")
set(FACE_UTILS_DIR  "${PROJECT_SOURCE_DIR}/utils")
# face morph model
add_executable(${FACE_MORPH}
    # utils
    ${FACE_UTILS_DIR}/landmarks.cpp
    # morph
    ${FACE_MORPH_DIR}/main.cpp
    ${FACE_MORPH_DIR}/morphable/basel.cpp
    ${FACE_MORPH_DIR}/visualization/window.cpp
)
target_link_libraries(${FACE_MORPH} snow)

# face alignment
add_executable(${FACE_ALIGN}
    # utils
    ${FACE_UTILS_DIR}/landmarks.cpp
    # morph
    ${FACE_ALIGN_DIR}/main.cpp
    ${FACE_ALIGN_DIR}/label/window.cpp
    ${FACE_ALIGN_DIR}/label/label_data.cpp
)
target_link_libraries(${FACE_ALIGN} snow)