cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    astar.h
    anim.h
    common.h
    file.h
    fsm.h
    objects.cpp
    objects.h
    random.h
    timer.h
    vec2.h
    vec3.h
    rect.h
    event.h
    buffer.h
    geometry.h
    particle.h
    mat2.h)

add_library(framework_core ${SOURCE_FILES})
