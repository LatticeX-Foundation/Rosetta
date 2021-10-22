cmake_minimum_required(VERSION 2.8)
project(zkp)

# public common link libraries
# list(APPEND LINKLIBS common)
# list(APPEND LINKLIBS mpc-io)

set(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

# zkp-comm
#add_subdirectory(zkp-common)

# zkp sub protocols (including internal tests)
add_subdirectory(mystique)

# mpc test, for all protocols (protocol-level)
#add_subdirectory(tests)

# generate all sub.so into one.so
option(GENERATE_ALL_IN_ONE "" OFF)
IF(GENERATE_ALL_IN_ONE)
    # TODO
    message(STATUS "Generate zkp.so")
ENDIF()

