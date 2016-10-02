cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test_core_bsp.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    SDL)

add_executable(test_core_bsp ${SOURCE_FILES})
target_link_libraries(test_core_bsp PUBLIC ${LIBRARIES})

set_target_properties(test_core_bsp PROPERTIES
    FOLDER tests
)
