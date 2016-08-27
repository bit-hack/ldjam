cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES objects.cpp objects.h spatial.cpp spatial.h random.h timer.h const.h vec2.h vec3.h)
add_library(framework ${SOURCE_FILES})
