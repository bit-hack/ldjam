cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test_spatial.cpp)

set(LIBS
    framework_core
    framework_draw
    framework_spatial
    SDL)
	
if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

add_executable(test_spatial ${SOURCE_FILES})
target_link_libraries(test_spatial PUBLIC ${LIBS})

set_target_properties(test_spatial PROPERTIES
    FOLDER tests
)
