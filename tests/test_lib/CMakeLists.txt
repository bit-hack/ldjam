cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# add test directory as define
add_definitions("-DTENGU_TEST_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")

set(SOURCE_FILES
    test_lib.h
    test_lib.cpp)

add_library(test_lib ${SOURCE_FILES})

set_target_properties(test_lib PROPERTIES
    FOLDER tests
)
