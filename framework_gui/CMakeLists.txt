cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    framework_gui.h
    framework_gui.cpp
    framework_gui_widgets.h
    framework_gui_widgets.cpp)

add_library(framework_gui ${SOURCE_FILES})

set_target_properties(framework_gui PROPERTIES
    FOLDER framework
)
