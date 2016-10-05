add_subdirectory(lib_vst)

file(GLOB H_FILES *.h)
file(GLOB CPP_FILES *.cpp)

add_library(vst_chip SHARED ${CPP_FILES} ${H_FILES})
target_link_libraries(vst_channel lib_vst framework_audio framework_core)

set_target_properties(vst_channel PROPERTIES
    FOLDER tools
)
