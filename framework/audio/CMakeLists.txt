cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES stb_vorbis.c stb_vorbis.h)
add_library(audio ${SOURCE_FILES})
