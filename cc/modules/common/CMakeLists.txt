cmake_minimum_required(VERSION 2.8)
project(common)

# External packages
if(UNIX)
  find_package(LibDL)
  find_package(LibRT)
endif()

# Link libraries
if(UNIX)
  list(APPEND LINKLIBS ${LIBDL_LIBRARIES})
  list(APPEND LINKLIBS ${LIBRT_LIBRARIES})
endif()

set(__SO_VERSION__ ${SO_VERSION})
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp"
)

#add_definitions(-DCOMMON_COMPILED_LIB)
SET(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

IF(USE_GMTASSL)
    # GMSSL
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/gmssl/include)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/gmssl/lib)
    link_libraries(tacrypto tassl)
ELSE()
    # OpenSSL
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_libraries(${OPENSSL_LIBRARIES})
ENDIF()

# Library
file(GLOB_RECURSE LIB_HEADER_FILES "include/*.h")
file(GLOB_RECURSE LIB_SOURCE_FILES "src/*.cpp")

set_source_files_properties(${LIB_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
add_library(common SHARED ${LIB_HEADER_FILES} ${LIB_SOURCE_FILES})
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/utils")
add_dependencies (common spdlog)
target_link_libraries(common ${LINKLIBS} common)
set_target_properties(common PROPERTIES FOLDER "common"
    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}")
list(APPEND LINKLIBS common)

if(COMMAND target_precompile_headers AND ROSETTA_ENABLE_PCH)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pch.h.in ${PROJECT_BINARY_DIR}/common_pch.h @ONLY)
  target_precompile_headers(common PRIVATE ${PROJECT_BINARY_DIR}/common_pch.h)
  message(STATUS "set PCH with common path: ${PROJECT_BINARY_DIR}/common_pch.h")
endif()

# Tests
IF(ROSETTA_COMPILE_TESTS)
  file(GLOB TESTS_HEADER_FILES "tests/*.h")
  file(GLOB TESTS_SOURCE_FILES "tests/*.cpp")
  set_source_files_properties(${TESTS_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}")
  add_executable(common-tests ${TESTS_HEADER_FILES} ${TESTS_SOURCE_FILES} ${Catch2})
  target_include_directories(common-tests PRIVATE ${Catch2})
  target_link_libraries(common-tests ${LINKLIBS})
  set_target_properties(common-tests PROPERTIES FOLDER tests)
ENDIF()
