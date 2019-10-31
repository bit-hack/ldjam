add_subdirectory(glee)

if (${USE_SPINE})
  add_subdirectory(spine-c)
endif()

option(USE_LIBSPNG "enable libspng library" OFF)

if (${USE_LIBSPNG})
  add_subdirectory(libspng-0.5.0)
endif()

option(USE_STB_IMAGE "enable libspng library" OFF)

if (${USE_STB_IMAGE})
  add_subdirectory(stb_image)
endif()
