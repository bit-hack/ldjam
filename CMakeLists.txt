cmake_minimum_required(VERSION 3.4)
project(tengu)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(USE_SPINE "enable spine-c runtime" OFF)
option(USE_FRAMEWORK_GUI "enable framework GUI" OFF)
option(USE_FRAMEWORK_AUDIO "enable framework audio" ON)
option(USE_FRAMEWORK_DRAW "enable framework draw" ON)
option(USE_FRAMEWORK_GLDRAW "enable framework gldraw" OFF)

if (${USE_FRAMEWORK_AUDIO})
  add_subdirectory(framework_audio)
endif()

add_subdirectory(framework_core)

if (${USE_FRAMEWORK_DRAW})
  add_subdirectory(framework_draw)
endif()

if (${USE_FRAMEWORK_GLDRAW})
  add_subdirectory(framework_gl_draw)
endif()

add_subdirectory(framework_sdl)
add_subdirectory(framework_spatial)
add_subdirectory(framework_tiles)

if (${USE_FRAMEWORK_GUI})
  add_subdirectory(framework_gui)
endif()

if (${USE_SPINE})
  add_subdirectory(framework_spine)
endif()

add_subdirectory(examples)
add_subdirectory(external)
add_subdirectory(tests)
