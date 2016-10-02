cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test_stb_vorbis.cpp)

add_executable(test_stb_vorbis ${SOURCE_FILES})
target_link_libraries(test_stb_vorbis framework_core framework_audio SDL)

set_target_properties(test_stb_vorbis PROPERTIES
    FOLDER tests
)
