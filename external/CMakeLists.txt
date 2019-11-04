add_subdirectory(glee)

option(USE_LIBSPNG "enable libspng library" OFF)
option(USE_STB_IMAGE "enable stb image library" OFF)
option(USE_STB_TTF "enable stb truetype library" OFF)

if (${USE_SPINE})
  add_subdirectory(spine-c)
endif()

if (${USE_LIBSPNG})
  add_subdirectory(libspng-0.5.0)
endif()


if (${USE_STB_IMAGE})
  add_subdirectory(stb_image)
endif()

if (${USE_STB_IMAGE})
  add_subdirectory(stb_ttf)
endif()
