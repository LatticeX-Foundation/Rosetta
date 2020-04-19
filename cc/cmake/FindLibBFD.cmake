# Try to find BFD library and include path.
# Once done this will define
#
# LIBBFD_FOUND
# LIBBFD_INCLUDE_DIR
# LIBBFD_LIBRARIES

find_path(LIBBFD_INCLUDE_DIR bfd.h)
find_library(LIBBFD_LIBRARY bfd)

# Handle the REQUIRED argument and set LIBBFD_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBBFD DEFAULT_MSG LIBBFD_LIBRARY LIBBFD_INCLUDE_DIR)

mark_as_advanced(LIBBFD_INCLUDE_DIR)
mark_as_advanced(LIBBFD_LIBRARY)

if(LIBBFD_FOUND)
  add_definitions(-DLIBBFD_SUPPORT)
  set(LIBBFD_LIBRARIES ${LIBBFD_LIBRARY})
endif()
