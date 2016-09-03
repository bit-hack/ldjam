cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    draw.cpp
    bitmap.cpp
    bitmap.h)

add_library(framework_draw ${SOURCE_FILES})
