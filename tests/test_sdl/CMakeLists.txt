cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# add test directory as define
add_definitions("-DTENGU_TEST_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")

if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

set(SOURCE_FILES
    test_sdl.h
    test_sdl.cpp)

add_library(test_sdl ${SOURCE_FILES})

set_target_properties(test_sdl PROPERTIES
    FOLDER tests
)
