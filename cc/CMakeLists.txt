cmake_minimum_required(VERSION 2.8)
project(rosetta)

if(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
    add_definitions(/W2)
endif()

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# setup directory where we should look for cmake modules files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GlobalSettings)


# deps (third_party: relic, emp-toolkit)
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/install_basic_deps_by_cmake.sh ${PROJECT_BINARY_DIR} ${CMAKE_BUILD_TYPE} ${THIRD_PARTY_DIR} 
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
                

# config mpc with 128-bits
option(ROSETTA_MPC_128 "" OFF)
if (ROSETTA_MPC_128)
    message(STATUS "ROSETTA_MPC_128: ${ROSETTA_MPC_128}, use 128-bits MPC")
    # add_compile_definitions(ROSETTA_MPC_128=1)
    add_definitions(-DROSETTA_MPC_128=1)
else()
    message(STATUS "ROSETTA_MPC_128: ${ROSETTA_MPC_128}, use 64-bits MPC")
endif()

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
option(ROSETTA_ENABLES_PROTOCOL_PSI "" OFF)
IF(ROSETTA_ENABLES_PROTOCOL_PSI)
    add_definitions(-DROSETTA_ENABLES_PROTOCOL_PSI=1)
ENDIF()
option(ROSETTA_ENABLES_PROTOCOL_ZK "" OFF)
IF(ROSETTA_ENABLES_PROTOCOL_ZK)
    add_definitions(-DROSETTA_ENABLES_PROTOCOL_ZK=1)
ENDIF()


# modules
add_subdirectory(modules)
add_subdirectory(tf)
