cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    atomic.h
    astar.h
    anim.h
    buffer.h
    common.h
    event.h
    file.h
    fsm.h
    geometry.h
    mat2.h
    objects.cpp
    objects.h
    particle.h
    random.h
    rect.h
	registry.h
    timer.h
    thread.h
    thread.cpp
    vec2.h
    vec3.h
    vec4.h
    string.h)

add_library(framework_core ${SOURCE_FILES})

set_target_properties (framework_core PROPERTIES
    FOLDER framework
)
