cmake_minimum_required(VERSION 3.4)
project(ldjam)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory(framework_core)
add_subdirectory(framework_audio)
add_subdirectory(framework_draw)
add_subdirectory(framework_gl_draw)
add_subdirectory(framework_tiles)
add_subdirectory(framework_spatial)

add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(tools)
