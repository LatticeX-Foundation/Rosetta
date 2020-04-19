find_path(MPIR_INCLUDE_DIR
        NAMES mpir.h
        HINTS "${MPIR_ROOT_DIR}/include"
        )

find_library(MPIR_LIBRARY
        NAMES mpir
        HINTS "${MPIR_ROOT_DIR}/lib"
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPIR DEFAULT_MSG MPIR_LIBRARY MPIR_INCLUDE_DIR)

mark_as_advanced(MPIR_LIBRARY MPIR_INCLUDE)

set(MPIR_LIBRARIES ${MPIR_LIBRARY})
set(MPIR_INCLUDE_DIRS ${MPIR_INCLUDE_DIR})
