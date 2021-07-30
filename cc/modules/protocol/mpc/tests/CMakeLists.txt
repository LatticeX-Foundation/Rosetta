cmake_minimum_required(VERSION 2.8)
project(mpc-tests)


SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")


IF(ROSETTA_COMPILE_TESTS)

# examples & tests
function(compile_mpc_protocol_test proto proj)
  string(TOUPPER ${proto} up_proto)
  set(projname protocol_mpc_tests_${proto}_${proj})
  add_executable(${projname} ${proj}.cpp)
  target_compile_definitions(${projname} PRIVATE PROTOCOL_MPC_TEST PRIVATE PROTOCOL_MPC_TEST_${up_proto}=1)
  target_link_libraries(${projname} mpc-${proto})# iowrapper)
endfunction()

# examples & tests for multiple tasks
function(compile_mpc_protocol_task_test proto proj)
  string(TOUPPER ${proto} up_proto)
  set(projname protocol_mpc_tests_${proto}_${proj})
  add_executable(${projname} tasks/${proj}.cpp)
  target_compile_definitions(${projname} PRIVATE PROTOCOL_MPC_TEST PRIVATE PROTOCOL_MPC_TEST_${up_proto}=1)
  target_link_libraries(${projname} mpc-${proto})# iowrapper)
endfunction()

# SecureNN single thread tests
IF(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN)
  compile_mpc_protocol_test(snn check)
  compile_mpc_protocol_test(snn perf)
  compile_mpc_protocol_test(snn matmul)
  compile_mpc_protocol_test(snn abs_ops)
  compile_mpc_protocol_test(snn logical_ops)
  compile_mpc_protocol_test(snn log_ops)
  compile_mpc_protocol_test(snn binary_ops)
  compile_mpc_protocol_test(snn unary_ops)
  compile_mpc_protocol_test(snn reduce_ops)
  compile_mpc_protocol_test(snn contrib_ops)
ENDIF()

# Helix single thread tests
IF(ROSETTA_ENABLES_PROTOCOL_MPC_HELIX)
  compile_mpc_protocol_test(helix check)
  compile_mpc_protocol_test(helix perf)
  compile_mpc_protocol_test(helix matmul)
  compile_mpc_protocol_test(helix abs_ops)
  compile_mpc_protocol_test(helix logical_ops)
  compile_mpc_protocol_test(helix log_ops)
  compile_mpc_protocol_test(helix binary_ops)
  compile_mpc_protocol_test(helix unary_ops)
  compile_mpc_protocol_test(helix reduce_ops)
  compile_mpc_protocol_test(helix contrib_ops)

ENDIF()

IF(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN)
  compile_mpc_protocol_test(naive simple_test)
ENDIF()

# SecureNN multiple task tests
IF(ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN)
  compile_mpc_protocol_task_test(snn abs_ops_tasks)
  compile_mpc_protocol_task_test(snn matmul_tasks)
  compile_mpc_protocol_task_test(snn binary_ops_tasks)
  compile_mpc_protocol_task_test(snn log_ops_tasks)
  compile_mpc_protocol_task_test(snn unary_ops_tasks)
  compile_mpc_protocol_task_test(snn reduce_ops_tasks)
  compile_mpc_protocol_task_test(snn logical_ops_tasks)
ENDIF()

# Helix multiple task tests
IF(ROSETTA_ENABLES_PROTOCOL_MPC_HELIX)
  compile_mpc_protocol_task_test(helix abs_ops_tasks)
  compile_mpc_protocol_task_test(helix matmul_tasks)
  compile_mpc_protocol_task_test(helix binary_ops_tasks)
  compile_mpc_protocol_task_test(helix log_ops_tasks)
  compile_mpc_protocol_task_test(helix unary_ops_tasks)
  compile_mpc_protocol_task_test(helix reduce_ops_tasks)
  compile_mpc_protocol_task_test(helix logical_ops_tasks)
ENDIF()

ENDIF() # ROSETTA_COMPILE_TESTS
