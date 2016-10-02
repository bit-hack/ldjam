file(GLOB H_FILES *.h)
file(GLOB CPP_FILES *.cpp)
add_library(lib_vst ${CPP_FILES} ${H_FILES})

set_target_properties(lib_vst PROPERTIES
    FOLDER tools
)
