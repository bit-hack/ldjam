cmake_minimum_required(VERSION 3.4)
project(tengu)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(USE_SPINE "enable spine-c runtime" OFF)

add_subdirectory(framework_audio)
add_subdirectory(framework_core)
add_subdirectory(framework_draw)
add_subdirectory(framework_gl_draw)
add_subdirectory(framework_sdl)
add_subdirectory(framework_spatial)
if (${USE_SPINE})
add_subdirectory(framework_spine)
endif()
add_subdirectory(framework_tiles)

add_subdirectory(examples)
add_subdirectory(external)
add_subdirectory(tests)
