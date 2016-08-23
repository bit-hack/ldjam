cmake_minimum_required(VERSION 3.6)
project(ldjam)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(framework)
add_subdirectory(testing)

set(SOURCE_FILES main.cpp)
add_executable(ldjam ${SOURCE_FILES})
target_link_libraries(ldjam PUBLIC framework)
