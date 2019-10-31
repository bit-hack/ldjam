cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    framework_spine.h
    framework_spine.cpp)

add_library(framework_spine ${SOURCE_FILES})

set_target_properties(framework_spine PROPERTIES
    FOLDER framework
)

target_link_libraries(framework_spine spine-c)
