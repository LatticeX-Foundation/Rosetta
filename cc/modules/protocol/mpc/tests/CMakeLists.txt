cmake_minimum_required(VERSION 2.8)
project(mpc-tests)


IF(ROSETTA_COMPILE_TESTS)
# examples & tests
function(compile_mpc_protocol_test proto proj)
  string(TOUPPER ${proto} up_proto)
  set(projname protocol_mpc_tests_${proto}_${proj})
  add_executable(${projname} ${proj}.cpp)
  target_compile_definitions(${projname} PRIVATE PROTOCOL_MPC_TEST PRIVATE PROTOCOL_MPC_TEST_${up_proto}=1)
  target_link_libraries(${projname} mpc-${proto})
endfunction()


IF(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN)
  compile_mpc_protocol_test(snn check)
  compile_mpc_protocol_test(snn perf)
  compile_mpc_protocol_test(snn op_logical)
ENDIF()

IF(ROSETTA_ENABLES_PROTOCOL_MPC_HELIX)
  compile_mpc_protocol_test(helix check)
  compile_mpc_protocol_test(helix perf)
  compile_mpc_protocol_test(helix op_logical)
ENDIF()

ENDIF()
