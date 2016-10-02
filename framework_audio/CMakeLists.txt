cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    audio.cpp
    audio.h
    queue.h)

set(SOURCE_FILES_FMT
    format/vorbis.h
    format/wave.cpp
    format/wave.h)

set(SOURCE_FILES_SRC
    source/vorbis/source_vorbis.cpp
    source/vorbis/source_vorbis.h
    source/vorbis/stb_vorbis.c
    source/vorbis/stb_vorbis.h
    source/wave/source_wave.cpp
    source/wave/source_wave.h
    source/chip/source_chip.cpp
    source/chip/source_chip.h
    source/chip/decimate.h
    source/chip/blip_table.cpp)

add_library(
    framework_audio 
    ${SOURCE_FILES}
    ${SOURCE_FILES_FMT}
    ${SOURCE_FILES_SRC})

set_target_properties (framework_audio PROPERTIES
    FOLDER framework
)
