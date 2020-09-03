cmake_minimum_required(VERSION 2.8)
project(mpc-comm)

# Common Sources
file(GLOB_RECURSE MPC_COMM_SOURCES_FILES "src/*.cpp")

# Library mpc-comm
add_library(mpc-comm SHARED ${MPC_COMM_SOURCES_FILES})
target_include_directories(mpc-comm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/3rd")
target_link_libraries(mpc-comm PUBLIC ${LINKLIBS} protocol-base)
set_target_properties(mpc-comm PROPERTIES FOLDER "protocol/comm"
  APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
install_libraries(mpc-comm)
