cmake_minimum_required(VERSION 3.4)

if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(SOURCE_FILES
    test_audio_unit.cpp)

add_executable(test_audio_unit ${SOURCE_FILES})
target_link_libraries(test_audio_unit PUBLIC framework_core framework_audio SDL)

set_target_properties(test_audio_unit PROPERTIES
    FOLDER tests
)
