cmake_minimum_required(VERSION 3.10)
#set(CMAKE_C_COMPILER ccache gcc)
#set(CMAKE_CXX_COMPILER ccache g++)
project(rosetta)

cmake_policy(SET CMP0069 NEW) 
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT _IsIPOSupported)
if(_IsIPOSupported)
    message(STATUS "Turn on INTERPROCEDURAL_OPTIMIZATION")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "Turn off INTERPROCEDURAL_OPTIMIZATION")
endif()

if(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
    add_definitions(/W2)
endif()

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

#set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions(-DSPDLOG_COMPILED_LIB)
#add_definitions(-fvisibility-inlines-hidden)
#add_definitions("-Wl,--threads -Wl,--thread-count,4")
#set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
#set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

# setup directory where we should look for cmake modules files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GlobalSettings)

# config mpc with 128-bits
option(ROSETTA_MPC_128 "" OFF)
if (ROSETTA_MPC_128)
    message(STATUS "ROSETTA_MPC_128: ${ROSETTA_MPC_128}, use 128-bits MPC")
    # add_compile_definitions(ROSETTA_MPC_128=1)
    add_definitions(-DROSETTA_MPC_128=1)
else()
    message(STATUS "ROSETTA_MPC_128: ${ROSETTA_MPC_128}, use 64-bits MPC")
endif()

# config shape inference
option(ROSETTA_ENABLES_SHAPE_INFERENCE "" OFF)
IF(ROSETTA_ENABLES_SHAPE_INFERENCE)
    add_definitions(-DROSETTA_ENABLES_SHAPE_INFERENCE=1)
ENDIF()

# ON OFF
# if COMPILE tests
option(ROSETTA_COMPILE_TESTS "" OFF)

# ON OFF
# if ENABLES protocol mpc/psi/zk/...
option(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN "" OFF) 
IF(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN)
    add_definitions(-DROSETTA_ENABLES_PROTOCOL_MPC_SECURENN=1)
ENDIF()
option(ROSETTA_ENABLES_PROTOCOL_MPC_HELIX "" OFF)
IF(ROSETTA_ENABLES_PROTOCOL_MPC_HELIX)
    add_definitions(-DROSETTA_ENABLES_PROTOCOL_MPC_HELIX=1)
ENDIF()
option(ROSETTA_ENABLES_PROTOCOL_ZK "" OFF)
IF(ROSETTA_ENABLES_PROTOCOL_ZK)
    add_definitions(-DROSETTA_ENABLES_PROTOCOL_ZK=1)
ENDIF()

# precompiled headers option
option(ROSETTA_ENABLE_PCH "using precompiled header to speed up compilation time" OFF)
if(NOT ${CMAKE_VERSION} VERSION_LESS 3.16)
    message(STATUS "cmake version is ${CMAKE_VERSION}")
    set(ROSETTA_ENABLE_PCH OFF)
    message(STATUS "set PCH with Rosetta")
endif()

# modules
add_subdirectory(third_party/spdlog)
add_subdirectory(modules)
add_subdirectory(tf)
add_subdirectory(third_party/io)
