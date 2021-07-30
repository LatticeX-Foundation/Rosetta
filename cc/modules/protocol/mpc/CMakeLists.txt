cmake_minimum_required(VERSION 2.8)
project(mpc)

# public common link libraries
list(APPEND LINKLIBS common)
#list(APPEND LINKLIBS mpc-io)

SET(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

# mpc-comm
add_subdirectory(comm)

# mpc sub protocols (including internal tests)
IF(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN)
    add_subdirectory(snn)
ENDIF()
IF(ROSETTA_ENABLES_PROTOCOL_MPC_HELIX)
    add_subdirectory(helix)
ENDIF()
add_subdirectory(naive)

# mpc test, for all protocols (protocol-level)
add_subdirectory(tests)

# generate all sub.so into one.so
option(GENERATE_ALL_IN_ONE "" OFF)
IF(GENERATE_ALL_IN_ONE)
    # TODO
    message(STATUS "Generate mpc.so")
ENDIF()

