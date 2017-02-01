cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# add test directory as define
add_definitions("-DTENGU_TEST_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")

set(SOURCE_FILES
    test.h
    test_tiles.cpp
    test_astar.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    framework_tiles
    SDL)
	
if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

add_executable(test_tiles ${SOURCE_FILES})
target_link_libraries(test_tiles PUBLIC ${LIBRARIES})

set_target_properties(test_tiles PROPERTIES
    FOLDER tests
)
