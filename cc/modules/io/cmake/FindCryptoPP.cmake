find_path(CRYPTOPP_INCLUDE_DIR
        NAMES cryptopp/cryptlib.h
        HINTS "${CRYPTOPP_ROOT_DIR}/include"
        )

find_library(CRYPTOPP_LIBRARY
        NAMES cryptopp
        HINTS "${CRYPTOPP_ROOT_DIR}/lib"
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CryptoPP REQUIRED_VARS CRYPTOPP_LIBRARY CRYPTOPP_INCLUDE_DIR)

mark_as_advanced(CRYPTOPP_INCLUDE_DIRS CRYPTOPP_LIBRARIES)

set(CRYPTOPP_LIBRARIES ${CRYPTOPP_LIBRARY})
set(CRYPTOPP_INCLUDE_DIRS ${CRYPTOPP_INCLUDE_DIR})
