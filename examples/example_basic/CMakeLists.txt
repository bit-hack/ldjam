cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    main.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    SDL)

add_executable(example_basic ${SOURCE_FILES})
target_link_libraries(example_basic ${LIBRARIES})
