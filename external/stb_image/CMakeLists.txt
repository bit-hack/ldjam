cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    stb_image.cpp
    stb_image.h)

add_library(stb_image ${SOURCE_FILES})

set_target_properties(stb_image PROPERTIES
    FOLDER external
)
