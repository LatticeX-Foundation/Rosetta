cmake_minimum_required(VERSION 2.8)
project(mpc-comm)

# Common Sources
file(GLOB_RECURSE MPC_COMM_SOURCES_FILES "src/*.cpp")

# Library mpc-comm
add_library(mpc-comm SHARED ${MPC_COMM_SOURCES_FILES})
include_directories("${CMAKE_BINARY_DIR}/include")
target_include_directories(mpc-comm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include")
target_link_libraries(mpc-comm PUBLIC ${LINKLIBS} iowrapper protocol-base protocol-utility)
set_target_properties(mpc-comm PROPERTIES FOLDER "protocol/comm"
  APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)

if(COMMAND target_precompile_headers AND ROSETTA_ENABLE_PCH)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pch.h.in ${PROJECT_BINARY_DIR}/comm_pch.h @ONLY)
  target_precompile_headers(mpc-comm PRIVATE ${PROJECT_BINARY_DIR}/comm_pch.h)
  message(STATUS "set PCH with mpc-comm path: ${PROJECT_BINARY_DIR}/comm_pch.h")
endif()

install_libraries(mpc-comm)
