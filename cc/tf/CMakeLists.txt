# tensorflow op libraries
cmake_minimum_required(VERSION 3.10)
project(tf)

SET(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
include_directories(${THIRD_PARTY_DIR}/spdlog-1.6.1/include)

add_subdirectory(rttops)
add_subdirectory(secureops)
add_subdirectory(dpass)
