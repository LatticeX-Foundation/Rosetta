find_package(emp-ot)

find_path(EMP-ZK_INCLUDE_DIR NAMES emp-zk/emp-zk.h)
find_library(EMP-ZK_LIBRARY NAMES emp-zk)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(emp-zk DEFAULT_MSG EMP-ZK_INCLUDE_DIR EMP-ZK_LIBRARY)

if(EMP-ZK_FOUND)
	set(EMP-ZK_INCLUDE_DIRS ${EMP-ZK_INCLUDE_DIR} ${EMP-OT_INCLUDE_DIRS})
	set(EMP-ZK_LIBRARIES ${EMP-OT_LIBRARIES} ${EMP-ZK_LIBRARY})
endif()
