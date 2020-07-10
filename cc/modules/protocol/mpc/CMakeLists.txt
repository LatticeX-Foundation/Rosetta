cmake_minimum_required(VERSION 3.10)
project(mpc)
include(GlobalSettings)

# public common link libraries
list(APPEND LINKLIBS common)
list(APPEND LINKLIBS mpc-io)

set(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

# mpc-comm
add_subdirectory(comm)

# mpc sub protocols
add_subdirectory(snn)

