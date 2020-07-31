cmake_minimum_required(VERSION 3.10)
project(mpc-snn)
include(GlobalSettings)

# SecureNN Operations
file(GLOB_RECURSE MPC_SNN_SOURCES_FILES "src/*.cpp")

# Library mpc-snn
add_library(mpc-snn SHARED  ${MPC_SNN_SOURCES_FILES})
target_include_directories(mpc-snn PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/snn"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/snn/io/include")
target_link_libraries(mpc-snn PUBLIC ${LINKLIBS} mpc-comm)
set_target_properties(mpc-snn PROPERTIES FOLDER "protocol/snn"
                    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
install_libraries(mpc-snn)

IF(ROSETTA_COMPILE_TESTS)
# examples & tests
function(compile_ex_snn category)
  file(GLOB EXAMPLE_SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/${category}" "${category}/*.cpp")
  foreach(EXAMPLE_SOURCE_FILE ${EXAMPLE_SOURCE_FILES})
    string(REGEX REPLACE "(.*)\\.cpp" "\\1" EXAMPLE_NAME ${EXAMPLE_SOURCE_FILE})
    set(EXAMPLE_TARGET "protocol_mpc_snn_${category}_${EXAMPLE_NAME}")
    add_executable(${EXAMPLE_TARGET} ${category}/${EXAMPLE_SOURCE_FILE})
    target_link_libraries(${EXAMPLE_TARGET} mpc-snn)
  endforeach()
endfunction()
compile_ex_snn(examples)
compile_ex_snn(tests)
ENDIF()
