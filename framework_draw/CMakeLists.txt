cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

file(GLOB SOURCE_FILES *.cpp *.h)

add_library(framework_draw ${SOURCE_FILES})

set_target_properties(framework_draw PROPERTIES
    FOLDER framework
)
