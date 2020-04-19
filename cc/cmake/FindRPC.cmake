# Try to find RPC library and include path.
# Once done this will define
#
# RPC_FOUND
# RPC_INCLUDE_DIR
# RPC_LIBRARIES

find_path(RPC_INCLUDE_DIR rpc.h)
if(MSVC)
  find_library(RPC_LIBRARY rpcrt4.lib)
else()
  find_library(RPC_LIBRARY rpcrt4)
endif()

# Handle the REQUIRED argument and set RPC_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RPC DEFAULT_MSG RPC_LIBRARY RPC_INCLUDE_DIR)

mark_as_advanced(RPC_INCLUDE_DIR)
mark_as_advanced(RPC_LIBRARY)

if(RPC_FOUND)
  add_definitions(-DRPC_SUPPORT)
  set(RPC_LIBRARIES ${RPC_LIBRARY})
endif()
