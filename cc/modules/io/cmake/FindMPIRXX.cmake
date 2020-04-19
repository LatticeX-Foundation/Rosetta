find_path(MPIRXX_INCLUDE_DIR
        NAMES mpirxx.h
        HINTS "${MPIR_ROOT_DIR}/include"
        )

find_library(MPIRXX_LIBRARY
        NAMES mpirxx
        HINTS "${MPIR_ROOT_DIR}/lib"
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPIRXX DEFAULT_MSG MPIRXX_LIBRARY MPIRXX_INCLUDE_DIR)

mark_as_advanced(MPIRXX_INCLUDE_DIR MPIRXX_LIBRARY)

set(MPIRXX_LIBRARIES ${MPIRXX_LIBRARY})
set(MPIRXX_INCLUDE_DIRS ${MPIRXX_INCLUDE_DIR})
