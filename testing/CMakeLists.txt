cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    testing.cpp
    testing.h
    test_objects.cpp
    test_spatial.cpp
    test_timer.cpp
    test_fsm.cpp)

add_executable(testing ${SOURCE_FILES})
target_link_libraries(testing PUBLIC framework SDL)
