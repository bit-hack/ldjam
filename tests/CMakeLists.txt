add_subdirectory(test_core_geometry)
add_subdirectory(test_core_rand)
add_subdirectory(test_core_unit)

add_subdirectory(test_lib)

if (${tengu_build_framework_audio})
  add_subdirectory(test_audio)
  add_subdirectory(test_audio_unit)
endif()

if (${tengu_build_framework_draw})
  add_subdirectory(test_draw)
  add_subdirectory(test_draw_unit)
endif()

if (${tengu_build_framework_gldraw})
  add_subdirectory(test_gl_draw)
endif()

if (${tengu_build_framework_spatial})
  add_subdirectory(test_spatial)
endif()

if (${tengu_build_framework_sdl})
  add_subdirectory(test_sdl)
endif()

add_subdirectory(test_stb_vorbis)

if (${tengu_build_framework_tiled})
  add_subdirectory(test_tiles)
endif()
