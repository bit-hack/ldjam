cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# add test directory as define
add_definitions("-DTENGU_TEST_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")

if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

set(SOURCE_FILES
    test_draw.cpp)

add_executable(test_draw ${SOURCE_FILES})
target_link_libraries(test_draw PUBLIC framework_core framework_draw SDL stb_ttf)

set_target_properties(test_draw PROPERTIES
    FOLDER tests
)
