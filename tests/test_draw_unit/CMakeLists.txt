cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test_bitmap.cpp
    test_draw_unit.cpp)

add_executable(test_draw_unit ${SOURCE_FILES})
target_link_libraries(test_draw_unit PUBLIC framework_core framework_draw SDL)

set_target_properties(test_draw_unit PROPERTIES
    FOLDER tests
)
