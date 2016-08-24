cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES objects.cpp objects.h spatial.cpp spatial.h)
add_library(framework ${SOURCE_FILES})
