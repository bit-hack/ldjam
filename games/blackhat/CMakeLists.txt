cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    computer.h
    computer.cpp
    main.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    SDL)

add_executable(games_blackhat ${SOURCE_FILES})
target_link_libraries(games_blackhat ${LIBRARIES})

set_target_properties(games_blackhat PROPERTIES
    FOLDER games)
