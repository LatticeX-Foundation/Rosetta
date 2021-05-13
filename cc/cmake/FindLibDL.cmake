# Try to find DL library and include path.
# Once done this will define
#
# LIBDL_FOUND
# LIBDL_INCLUDE_DIR
# LIBDL_LIBRARIES

find_path(LIBDL_INCLUDE_DIR dlfcn.h)
find_library(LIBDL_LIBRARY dl)

# Handle the REQUIRED argument and set LIBDL_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBDL DEFAULT_MSG LIBDL_LIBRARY LIBDL_INCLUDE_DIR)

mark_as_advanced(LIBDL_INCLUDE_DIR)
mark_as_advanced(LIBDL_LIBRARY)

if(LIBDL_FOUND)
  add_definitions(-DLIBDL_SUPPORT)
  set(LIBDL_LIBRARIES ${LIBDL_LIBRARY})
endif()
