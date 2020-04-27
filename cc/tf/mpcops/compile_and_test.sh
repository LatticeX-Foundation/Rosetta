#!/bin/bash

# This script for compiling forward OPs and back-propagation OPs.

set -x

curdir=$(pwd)
ccdir=${curdir}/../../
builddir=${ccdir}/../build/
libpath=${builddir}/lib
mkdir -p ${libpath}

TF_INC=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_include())')
TF_LIB=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_lib())')
TF_CFLGS=$(python3 -c 'import tensorflow as tf; print(" ".join(tf.sysconfig.get_compile_flags()))')
TF_LFLG=$(python3 -c 'import tensorflow as tf; print(" ".join(tf.sysconfig.get_link_flags()))')

#TF_CFLGS="-D_GLIBCXX_USE_CXX11_ABI=1"

# mpc includes and libs
MPC_INC=" -DSML_USE_UINT64=1"
MPC_INC+=" -I ${ccdir}/modules/common/include"
MPC_INC+=" -I ${ccdir}/modules/common/include/utils"
MPC_INC+=" -I ${ccdir}/modules/io/include"
MPC_INC+=" -I ${ccdir}/modules/protocol/mpc/include"
MPC_INC+=" -I ${ccdir}/modules/protocol/mpc/src/snn"
MPC_INC+=" -I ${ccdir}/third_party/rapidjson/include"

MPC_LIB=" -L ${builddir}/lib -lmpc-op"

# print all ops
# grep REGISTER_OP ops/*.cc

#
## compile libtf-mpcop.so
#
cd ${curdir}/ops
cflags=" -g "
cflags=" -O2 "
g++ ${cflags} -std=c++11 -shared *.cc -o ${libpath}/libtf-mpcop.so ${TF_CFLGS} \
    -I. -I ${TF_INC} ${MPC_INC} ${MPC_LIB} -L ${TF_LIB} ${TF_LFLG} -fPIC -Wl,--rpath='$ORIGIN/..:$ORIGIN'
cd ${curdir}

# for gradient ops here

# for pass here
