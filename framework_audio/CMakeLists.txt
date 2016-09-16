cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    audio.cpp
    audio.h
    stb_vorbis.c
    stb_vorbis.h
    wave.cpp
    wave.h
    vorbis.h
    queue.h source_wave.h source_vorbis.h source_wave.cpp source_vorbis.cpp)

add_library(framework_audio ${SOURCE_FILES})
