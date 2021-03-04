cmake_minimum_required(VERSION 2.8)
project(protocol-utility)

# Common Sources
file(GLOB_RECURSE MPC_COMM_SOURCES_FILES "src/*.cpp")

# Library protocol-utility
add_library(protocol-utility SHARED ${MPC_COMM_SOURCES_FILES})
include_directories("${CMAKE_BINARY_DIR}/include")
target_include_directories(protocol-utility PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include")
target_link_libraries(protocol-utility PUBLIC ${LINKLIBS} protocol-base)
set_target_properties(protocol-utility PROPERTIES FOLDER "protocol/comm"
  APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
install_libraries(protocol-utility)
