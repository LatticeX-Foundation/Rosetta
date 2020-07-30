# Set tensorflow compile options

# C++ standard
set(CMAKE_CXX_STANDARD 11)

# tensorflow compile flags with include directory
execute_process(
    COMMAND python3 -c "import tensorflow as tf; print(' '.join(tf.sysconfig.get_compile_flags()), end='')"
    OUTPUT_VARIABLE TF_CFLAGS
)
message(STATUS "TF_CFLAGS: " ${TF_CFLAGS})

# tensorflow include directory
execute_process(
    COMMAND python3 -c "import tensorflow as tf; print(tf.sysconfig.get_include(), end='')"
    OUTPUT_VARIABLE TF_INC
)
message(STATUS "TF_INC: " ${TF_INC})
set(TF_EIGEN3 ${TF_INC}/third_party/eigen3)

# tensorflow link flags with link path
execute_process(
    COMMAND python3 -c "import tensorflow as tf; print(' '.join(tf.sysconfig.get_link_flags()), end='')"
    OUTPUT_VARIABLE TF_LD_FLAGS
)
message(STATUS "TF_LD_FLAGS: " ${TF_LD_FLAGS})

# append flags
set(RUNTIME_FLAGS "-Wl,--rpath='$ORIGIN'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TF_CFLAGS} ${TF_LD_FLAGS} ${RUNTIME_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TF_CFLAGS} ${TF_LD_FLAGS} ${RUNTIME_FLAGS}")

include_directories(${TF_INC})
include_directories(${TF_EIGEN3})