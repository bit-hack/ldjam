cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

set(SOURCE_FILES
    common.h
    main.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    framework_audio
    framework_tiles
    SDL)

add_executable(example_racer ${SOURCE_FILES})
target_link_libraries(example_racer ${LIBRARIES})

set_target_properties (example_racer PROPERTIES
    FOLDER examples
)
