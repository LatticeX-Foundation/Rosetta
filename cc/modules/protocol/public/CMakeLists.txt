cmake_minimum_required(VERSION 2.8)
project(protocol_api)

# Link libraries
list(APPEND LINKLIBS common)
#list(APPEND LINKLIBS mpc-io)

# settings
include(SetCommon)

# Source files
# file(GLOB_RECURSE PUBLIC_SOURCES_FILES "src/*.cpp")

#TODO: try modifing this to fix cycle-dependency.
set(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

# Library protocol-base
add_library(protocol-base SHARED "src/protocol_base.cpp" "src/protocol_ops.cpp")
target_include_directories(protocol-base PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(protocol-base ${LINKLIBS})
set_target_properties(protocol-base PROPERTIES FOLDER "protocol/base"
                    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
install_libraries(protocol-base)

list(APPEND LINKLIBS protocol-base)
IF(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN)
list(APPEND LINKLIBS mpc-snn)
ENDIF()
IF(ROSETTA_ENABLES_PROTOCOL_MPC_HELIX)
list(APPEND LINKLIBS mpc-helix)
ENDIF()
list(APPEND LINKLIBS mpc-naive)
IF(ROSETTA_ENABLES_PROTOCOL_ZK)
list(APPEND LINKLIBS zkp-wolverine)
ENDIF()
# Library protocol-api
add_library(protocol-api SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol_manager.cpp)
target_include_directories(protocol-api PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../mpc/helix/include"
                        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../zk/wolverine/include")
target_link_libraries(protocol-api ${LINKLIBS})
set_target_properties(protocol-api PROPERTIES FOLDER "protocol/api"
                    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
install_libraries(protocol-api)


