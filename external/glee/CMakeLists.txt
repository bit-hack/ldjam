cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    GLee.c
    GLee.h)

add_library(glee ${SOURCE_FILES})

set_target_properties(glee PROPERTIES
    FOLDER external
)
