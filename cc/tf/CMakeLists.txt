# tensorflow op libraries
cmake_minimum_required(VERSION 2.8)
project(tf)

SET(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
include_directories(${THIRD_PARTY_DIR}/spdlog/include)

add_subdirectory(rttops)
add_subdirectory(secureops)
add_subdirectory(dpass)
