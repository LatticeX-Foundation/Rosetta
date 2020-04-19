# Try to find UUID library and include path.
# Once done this will define
#
# LIBUUID_FOUND
# LIBUUID_INCLUDE_DIR
# LIBUUID_LIBRARIES

find_path(LIBUUID_INCLUDE_DIR uuid/uuid.h)
find_library(LIBUUID_LIBRARY uuid)

# Handle the REQUIRED argument and set LIBUUID_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBUUID DEFAULT_MSG LIBUUID_LIBRARY LIBUUID_INCLUDE_DIR)

mark_as_advanced(LIBUUID_INCLUDE_DIR)
mark_as_advanced(LIBUUID_LIBRARY)

if(LIBUUID_FOUND)
  add_definitions(-DLIBUUID_SUPPORT)
  set(LIBUUID_LIBRARIES ${LIBUUID_LIBRARY})
endif()
