cmake_minimum_required(VERSION 3.10)
project(tf-dpass)
set(NAME tf-dpass)

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

#include_directories(${THIRD_PARTY_DIR}) # for catch2, cmdline
#include_directories(${THIRD_PARTY_DIR}/rapidjson/include)
#include_directories(${THIRD_PARTY_DIR}/pybind11/include)


message(STATUS "----MPC CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "rapidjson/include: ${THIRD_PARTY_DIR}/rapidjson/include")

# include
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/..)

# links 
message(STATUS "lib:  ${CMAKE_INSTALL_PREFIX}/lib")
link_directories(${CMAKE_INSTALL_PREFIX}/lib)

# External packages
find_package(Threads REQUIRED)
if(UNIX)
  find_package(LibDL)
  find_package(LibRT)
endif()

# Link libraries
list(APPEND LINKLIBS Threads::Threads)
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

add_library(tf-dpass SHARED ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
#target_include_directories(tf-dpass PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/utils")
target_link_libraries(tf-dpass ${LINKLIBS})

install_libraries(tf-dpass)