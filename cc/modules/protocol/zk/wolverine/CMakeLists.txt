cmake_minimum_required(VERSION 2.8)
SET(LIBNAME zkp-wolverine)
project(${LIBNAME})


# threads
find_package(Threads REQUIRED)


# OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_libraries(${OPENSSL_LIBRARIES})

# emp-toolkit
find_package(emp-tool REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS})

find_package(emp-ot REQUIRED)
include_directories(${EMP-OT_INCLUDE_DIRS})

# find_package(emp-wolverine-bool REQUIRED)
# include_directories(${EMP-WOLVERINE-BOOL_INCLUDE_DIRS})

# find_package(emp-wolverine-fp REQUIRED)
# include_directories(${EMP-WOLVERINE-FP_INCLUDE_DIRS})

find_package(emp-zk REQUIRED)
include_directories(${EMP-ZK_INCLUDE_DIRS})

# setting 
include(SetCommon)
include(SetCompileOptions)

# ####################################
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native")

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/wolverine_core/arithm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#link_libraries(${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES} ${EMP-WOLVERINE-BOOL_LIBRARIES} ${EMP-WOLVERINE-FP_LIBRARIES})
link_libraries(${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES} ${EMP-ZK_LIBRARIES})

# Library zkp-wolverine
file(GLOB_RECURSE MPC_WVR_SOURCES_FILES "src/*.cpp")
add_library(zkp-wolverine SHARED ${MPC_WVR_SOURCES_FILES})
target_include_directories(zkp-wolverine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
#target_link_libraries(zkp-wolverine PUBLIC ${LINKLIBS} protocol-utility ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES} ${EMP-WOLVERINE-BOOL_LIBRARIES} ${EMP-WOLVERINE-FP_LIBRARIES})
target_link_libraries(zkp-wolverine PUBLIC ${LINKLIBS} protocol-utility ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES} ${EMP-ZK_LIBRARIES})

set_target_properties(zkp-wolverine PROPERTIES FOLDER "protocol/wolverine")
set_target_properties(zkp-wolverine PROPERTIES FOLDER "protocol/wolverine"
                        APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
install_libraries(zkp-wolverine)


IF(ROSETTA_COMPILE_TESTS)
function(compile_ex_zkp category)
file(GLOB EXAMPLE_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/${category}" "${category}/*.cpp")
  foreach(EXAMPLE_SOURCE_FILE ${EXAMPLE_SOURCE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" EXAMPLE_NAME ${EXAMPLE_SOURCE_FILE})
    set(EXAMPLE_TARGET "protocol_zkp_wolverine_${category}_${EXAMPLE_NAME}")
    add_executable(${EXAMPLE_TARGET} ${category}/${EXAMPLE_SOURCE_FILE})
    #target_link_libraries(${EXAMPLE_TARGET} zkp-wolverine protocol-utility ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES} ${EMP-WOLVERINE-BOOL_LIBRARIES} ${EMP-WOLVERINE-FP_LIBRARIES})
    target_link_libraries(${EXAMPLE_TARGET} zkp-wolverine protocol-utility ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES} ${EMP-ZK_LIBRARIES})
  endforeach()
endfunction()
compile_ex_zkp(tests)
ENDIF()
