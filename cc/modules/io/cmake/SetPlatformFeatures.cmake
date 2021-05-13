# Platform features

if(CYGWIN)
  # Base Windows platform
  add_definitions(-DWIN32)

  # Windows 10
  add_definitions(-D_WIN32_WINNT=0x0A00)
elseif(WIN32)
  add_definitions(-DWIN32)
  add_definitions(-D_WIN32_WINNT=0x0603)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

  # Disable CRT secure warnings
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

  # Disable C++17 deprecation warnings
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

  # Windows 10
  add_definitions(-D_WIN32_WINNT=0x0A00)

  # Windows SDK
  if(MINGW OR CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION STREQUAL "")
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" CMAKE_WIN32_SDK ${CMAKE_SYSTEM_VERSION})
  else()
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" CMAKE_WIN32_SDK ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
  endif()
  add_definitions(-D_WIN32_SDK=${CMAKE_WIN32_SDK})
endif()
