cmake_minimum_required(VERSION 3.4)
project(ldjam)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(framework)
add_subdirectory(testing)
add_subdirectory(game_surge)

set(SOURCE_FILES main.cpp)
add_executable(ldjam ${SOURCE_FILES})
target_link_libraries(ldjam PUBLIC framework)
