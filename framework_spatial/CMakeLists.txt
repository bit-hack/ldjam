cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    space_hash.h
    space_hash.cpp
    sweep_prune.h aabb_tree.h aabb_tree.cpp)

set(${LIBS}
    framework_core)

add_library(framework_spatial ${SOURCE_FILES})
target_link_libraries(framework_spatial ${LIBS})
