cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    anim.h
    common.h
    file.h
    fsm.h
    objects.cpp
    objects.h
    random.h
    spatial.cpp
    spatial.h
    timer.h
    vec2.h
    vec3.h
    rect.h
    event.h
    buffer.h
    geometry.h
    bsp.h
    bsp.cpp
    particle.h)

add_library(framework_core ${SOURCE_FILES})
