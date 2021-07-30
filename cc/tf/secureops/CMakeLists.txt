cmake_minimum_required(VERSION 2.8)
project(secure-ops)

# settings
include(SetCommon)
#include(SetFunctions)
#include(SetPlatformFeatures)
#include(SetCompileOptions)
#include(SetVersionNumber)
include(SetTfCompileOptions)

# third party libraries include
# default directory

SET(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
message(STATUS "THIRD_PARTY_DIR:  ${THIRD_PARTY_DIR}")

include_directories(${THIRD_PARTY_DIR}) # for catch2, cmdline
include_directories(${THIRD_PARTY_DIR}/rapidjson/include)
include_directories(${THIRD_PARTY_DIR}/pybind11/include)


# include
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deps)

# links 
message(STATUS "lib:  ${CMAKE_INSTALL_PREFIX}/lib")
link_directories(${CMAKE_INSTALL_PREFIX}/lib)

# External packages
if(UNIX)
  find_package(LibDL)
  find_package(LibRT)
endif()

# Link libraries
if(UNIX)
  list(APPEND LINKLIBS ${LIBDL_LIBRARIES})
  list(APPEND LINKLIBS ${LIBRT_LIBRARIES})
  list(APPEND LINKLIBS ${TF_LD_FLAGS})
  list(APPEND LINKLIBS protocol-api)
#  list(APPEND LINKLIBS mpc-io)
  list(APPEND LINKLIBS protocol-base)
  list(APPEND LINKLIBS ${TF_LD_FLAGS})
endif()

message(STATUS "TF_LD_FLAGS :  ${TF_LD_FLAGS}")

# Library
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE LIB_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE LIB_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(secure-ops SHARED ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
#target_include_directories(secure-ops PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/utils")
#target_link_libraries(secure-ops ${LINKLIBS})
#message(STATUS "=============== secure-ops cmake ${LINKLIBS}")
add_dependencies(secure-ops common)
target_link_libraries(secure-ops ${LINKLIBS} common)

if(COMMAND target_precompile_headers AND ROSETTA_ENABLE_PCH)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pch.h.in ${PROJECT_BINARY_DIR}/secureops_pch.h @ONLY)
  target_precompile_headers(secure-ops PRIVATE ${PROJECT_BINARY_DIR}/secureops_pch.h)
  message(STATUS "set PCH with secure-ops path: ${PROJECT_BINARY_DIR}/secureops_pch.h")
endif()

install_libraries(secure-ops)
