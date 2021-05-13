# Try to find Crypt library and include path.
# Once done this will define
#
# CRYPT_FOUND
# CRYPT_INCLUDE_DIR
# CRYPT_LIBRARIES

find_path(CRYPT_INCLUDE_DIR wincrypt.h)
if(MSVC)
  find_library(CRYPT_LIBRARY crypt32.lib)
else()
  find_library(CRYPT_LIBRARY crypt32)
endif()

# Handle the REQUIRED argument and set CRYPT_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Crypt DEFAULT_MSG CRYPT_LIBRARY CRYPT_INCLUDE_DIR)

mark_as_advanced(CRYPT_INCLUDE_DIR)
mark_as_advanced(CRYPT_LIBRARY)

if(CRYPT_FOUND)
  add_definitions(-DCRYPT_SUPPORT)
  set(CRYPT_LIBRARIES ${CRYPT_LIBRARY})
endif()
