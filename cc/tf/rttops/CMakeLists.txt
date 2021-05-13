cmake_minimum_required(VERSION 2.8)
project(rtt-ops)

# System information
message("---> The System identification is ${CMAKE_SYSTEM} ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_INFO_FILE}")

# settings
include(SetCommon)
#include(SetFunctions)
#include(SetPlatformFeatures)
#include(SetCompileOptions)
#include(SetVersionNumber)
include(SetTfCompileOptions)

message(STATUS "----MPC CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# include
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../deps)

# links 
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
endif()

message(STATUS "TF_LD_FLAGS :  ${TF_LD_FLAGS}")

# Library
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB_RECURSE LIB_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE LIB_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(rtt-ops SHARED ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
#target_include_directories(rtt-ops PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/utils")
target_link_libraries(rtt-ops ${LINKLIBS})

install_libraries(rtt-ops)
