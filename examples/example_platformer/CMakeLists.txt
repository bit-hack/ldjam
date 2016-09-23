cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    main.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    framework_audio
    framework_tiles
    SDL)

add_executable(example_platformer ${SOURCE_FILES})
target_link_libraries(example_platformer ${LIBRARIES})
