cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test_core_spatial.cpp)

add_executable(test_core_spatial ${SOURCE_FILES})
target_link_libraries(test_core_spatial PUBLIC framework_core SDL)
