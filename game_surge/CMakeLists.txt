cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(game_surge ${SOURCE_FILES})
target_link_libraries(game_surge PUBLIC framework SDL2)
