# Try to find RT library and include path.
# Once done this will define
#
# LIBRT_FOUND
# LIBRT_INCLUDE_DIR
# LIBRT_LIBRARIES

find_path(LIBRT_INCLUDE_DIR time.h)
find_library(LIBRT_LIBRARY rt)

# Handle the REQUIRED argument and set LIBRT_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBRT DEFAULT_MSG LIBRT_LIBRARY LIBRT_INCLUDE_DIR)

mark_as_advanced(LIBRT_INCLUDE_DIR)
mark_as_advanced(LIBRT_LIBRARY)

if(LIBRT_FOUND)
  add_definitions(-DLIBRT_SUPPORT)
  set(LIBRT_LIBRARIES ${LIBRT_LIBRARY})
endif()
