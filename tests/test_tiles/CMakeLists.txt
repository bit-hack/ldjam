cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test.h
    test_tiles.cpp
    test_astar.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    framework_tiles
    SDL)

add_executable(test_tiles ${SOURCE_FILES})
target_link_libraries(test_tiles PUBLIC ${LIBRARIES})
