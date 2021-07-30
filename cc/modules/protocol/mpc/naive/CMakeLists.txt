cmake_minimum_required(VERSION 2.8)
project(mpc-naive)

file(GLOB_RECURSE MPC_NAIVE_SOURCES_FILES "src/*.cpp")

# Library mpc-naive
add_library(mpc-naive SHARED ${MPC_NAIVE_SOURCES_FILES})
target_include_directories(mpc-naive PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mpc-naive PUBLIC ${LINKLIBS} mpc-comm)
set_target_properties(mpc-naive PROPERTIES FOLDER "protocol/naive"
                    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)

if(COMMAND target_precompile_headers AND ROSETTA_ENABLE_PCH)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pch.h.in ${PROJECT_BINARY_DIR}/mpc_native_pch.h @ONLY)
  target_precompile_headers(mpc-naive PRIVATE ${PROJECT_BINARY_DIR}/mpc_native_pch.h)
  message(STATUS "set PCH with mpc-naive path: ${PROJECT_BINARY_DIR}/mpc_native_pch.h")
endif()

install_libraries(mpc-naive)


# IF(ROSETTA_COMPILE_TESTS)
# # examples & tests
# function(compile_ex_naive category)
# file(GLOB EXAMPLE_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/${category}" "${category}/*.cpp")
#   foreach(EXAMPLE_SOURCE_FILE ${EXAMPLE_SOURCE_FILES})
#     string(REGEX REPLACE "(.*)\\.cpp" "\\1" EXAMPLE_NAME ${EXAMPLE_SOURCE_FILE})
#     set(EXAMPLE_TARGET "protocol_mpc_naive_${category}_${EXAMPLE_NAME}")
#     add_executable(${EXAMPLE_TARGET} ${category}/${EXAMPLE_SOURCE_FILE})
#     target_link_libraries(${EXAMPLE_TARGET} mpc-naive)
#   endforeach()
# endfunction()
# compile_ex_naive(examples)
# compile_ex_naive(tests)
# ENDIF()
