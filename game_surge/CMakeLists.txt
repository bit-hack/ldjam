cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp draw.cpp draw.h surge.cpp surge.h)
add_executable(game_surge ${SOURCE_FILES})
target_link_libraries(game_surge PUBLIC framework SDL2)
