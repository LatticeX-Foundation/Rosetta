IF(NOT TARGET io)
  # You can use io based on libevent to set USE_LIBEVENT ON
  option(USE_LIBEVENT "" OFF)
  if(USE_LIBEVENT)
    add_definitions(-DUSE_LIBEVENT)
  endif()
  add_subdirectory(io)
ENDIF()
