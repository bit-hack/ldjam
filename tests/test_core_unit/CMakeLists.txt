cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test_core_anim.cpp
    test_core_file.cpp
    test_core_fsm.cpp
    test_core_object.cpp
    test_core_timer.cpp
    test_core.cpp)

add_executable(test_core_unit ${SOURCE_FILES})
target_link_libraries(test_core_unit PUBLIC framework_core SDL)
