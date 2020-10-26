cmake_minimum_required(VERSION 2.8)
project(mpc)

# public common link libraries
list(APPEND LINKLIBS common)
list(APPEND LINKLIBS mpc-io)

set(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

# mpc-comm
add_subdirectory(comm)

# mpc sub protocols (including internal tests)
add_subdirectory(snn)
add_subdirectory(naive)

# mpc test, for all protocols (protocol-level)
add_subdirectory(tests)
