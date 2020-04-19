cmake_minimum_required(VERSION 3.10)
project(common)
include(GlobalSettings)

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
endif()

# Library
file(GLOB_RECURSE LIB_HEADER_FILES "include/*.h")
file(GLOB_RECURSE LIB_SOURCE_FILES "src/*.cpp")
set_source_files_properties(${LIB_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
add_library(common ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/utils")
target_link_libraries(common ${LINKLIBS})
set_target_properties(common PROPERTIES FOLDER libraries)
list(APPEND LINKLIBS common)

# Tests
file(GLOB TESTS_HEADER_FILES "tests/*.h")
file(GLOB TESTS_SOURCE_FILES "tests/*.cpp")
set_source_files_properties(${TESTS_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
add_executable(common-tests ${TESTS_HEADER_FILES} ${TESTS_SOURCE_FILES} ${Catch2})
target_include_directories(common-tests PRIVATE ${Catch2})
target_link_libraries(common-tests ${LINKLIBS})
set_target_properties(common-tests PROPERTIES FOLDER tests)
