cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    gl_draw.cpp
    gl_draw.h
    gl_shader.h
    gl_texture.h
    gl_mat4.h
    gl_shader.cpp
    gl_texture.cpp)

add_library(framework_gl_draw ${SOURCE_FILES})

set_target_properties(framework_gl_draw PROPERTIES
    FOLDER framework
)
