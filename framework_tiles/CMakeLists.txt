cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    tiles.h
    tiles.cpp)

set(${LIBS}
    framework_core)

add_library(framework_tiles ${SOURCE_FILES})
target_link_libraries(framework_tiles ${LIBS})
