cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if (MSVC)
    add_definitions(-D_SDL_main_h)
endif()

set(SOURCE_FILES
    main.cpp)

set(LIBRARIES
    framework_core
    framework_draw
    framework_gui
    SDL)

add_executable(example_gui ${SOURCE_FILES})
target_link_libraries(example_gui ${LIBRARIES})

set_target_properties (example_gui PROPERTIES
    FOLDER examples
)
