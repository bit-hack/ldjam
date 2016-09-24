cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    camera.h
    camera.cpp
    player.h
    player.cpp
    main.cpp
    common.h particles.h)

set(LIBRARIES
    framework_core
    framework_draw
    framework_audio
    framework_tiles
    SDL)

add_executable(example_platformer ${SOURCE_FILES})
target_link_libraries(example_platformer ${LIBRARIES})
