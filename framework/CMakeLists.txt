cmake_minimum_required(VERSION 3.4)

add_subdirectory(audio)
add_subdirectory(draw)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    anim.h
    const.h
    fsm.h
    objects.cpp
    objects.h
    random.h
    spatial.cpp
    spatial.h
    timer.h
    vec2.h
    vec3.h
    file.h)

add_library(framework ${SOURCE_FILES})
