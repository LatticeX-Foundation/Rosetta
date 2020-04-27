cmake_minimum_required(VERSION 3.10)
project(mpc)
include(GlobalSettings)

# defines for secure-nn
add_definitions(-DSML_USE_UINT64=1)
add_definitions(-DFLOAT_PRECISION=13)

# Link libraries
list(APPEND LINKLIBS common)
list(APPEND LINKLIBS mpc-io)

# MPC operations
file(GLOB_RECURSE MPC_SOURCES_FILES "src/ops/*.cpp")

# SecureNN Operations
file(GLOB_RECURSE SNN_SOURCES_FILES "src/snn/*.cpp")

# Rosetta Operations
file(GLOB_RECURSE RTT_SOURCES_FILES "src/rtt/*.cpp")

set(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

# Library
add_library(mpc-op SHARED ${MPC_SOURCES_FILES} ${SNN_SOURCES_FILES} ${RTT_SOURCES_FILES})
target_include_directories(mpc-op PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/snn"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/snn/io/include")
target_link_libraries(mpc-op ${LINKLIBS})
set_target_properties(mpc-op PROPERTIES FOLDER "protocol/mpc"
                    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
list(APPEND LINKLIBS mpc-op)
install_libraries(mpc-op)

# tests
set(proj protocol_mpc_SNN)
file(GLOB srcss tests/*.cpp)
add_executable(${proj} ${srcss})
target_include_directories(${proj} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/snn"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/snn/io/include")
target_link_libraries(${proj} mpc-op) 
