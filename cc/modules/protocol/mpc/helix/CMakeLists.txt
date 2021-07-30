cmake_minimum_required(VERSION 2.8)
project(mpc-helix)

file(GLOB_RECURSE MPC_HELIX_SOURCES_FILES "src/*.cpp")

include_directories("${CMAKE_BINARY_DIR}/include")

# Library mpc-helix
add_library(mpc-helix SHARED ${MPC_HELIX_SOURCES_FILES})
target_include_directories(mpc-helix PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mpc-helix PUBLIC ${LINKLIBS} mpc-comm)
set_target_properties(mpc-helix PROPERTIES FOLDER "protocol/helix"
                    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)

if(COMMAND target_precompile_headers AND ROSETTA_ENABLE_PCH)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pch.h.in ${PROJECT_BINARY_DIR}/mpc_helix_pch.h @ONLY)
  target_precompile_headers(mpc-helix PRIVATE ${PROJECT_BINARY_DIR}/mpc_helix_pch.h)
  message(STATUS "set PCH with mpc-helix path: ${PROJECT_BINARY_DIR}/mpc_helix_pch.h")
endif()

install_libraries(mpc-helix)

IF(ROSETTA_COMPILE_TESTS)
# examples & tests
function(compile_ex_helix category)
  file(GLOB EXAMPLE_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/${category}" "${category}/*.cpp")
  foreach(EXAMPLE_SOURCE_FILE ${EXAMPLE_SOURCE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" EXAMPLE_NAME ${EXAMPLE_SOURCE_FILE})
    set(EXAMPLE_TARGET "protocol_mpc_helix_${category}_${EXAMPLE_NAME}")
    add_executable(${EXAMPLE_TARGET} ${category}/${EXAMPLE_SOURCE_FILE})
    target_link_libraries(${EXAMPLE_TARGET} mpc-helix)
  endforeach()
endfunction()
compile_ex_helix(examples)
compile_ex_helix(tests)
ENDIF()
