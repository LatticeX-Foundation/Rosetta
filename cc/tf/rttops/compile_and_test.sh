#!/bin/bash

set -x

curdir=$(pwd)
ccdir=${curdir}/../../
builddir=${ccdir}/../build/
libpath=${builddir}/lib
if [ ! -e ${libpath} ]; then
    mkdir -p ${libpath}
fi


echo "compiling rtt_ops.so..."

#
## compile rrt_op.so
#
cd ${curdir}
CFLAGS=" -Wno-format-overflow -g "

libname="librtt_ops.so"

TF_INC=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_include())')
TF_LIB=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_lib())')
TF_CFLG=$(python3 -c 'import tensorflow as tf; print(" ".join(tf.sysconfig.get_compile_flags()))')
TF_LFLG=$(python3 -c 'import tensorflow as tf; print(" ".join(tf.sysconfig.get_link_flags()))')

CC_INC="-I./ -I../mpcops/ops/thirdparty -I ${TF_INC} -I../mpcops/ops"
CC_FLAGS="${CFLAGS} ${TF_CFLG}"
#########################
## compile rtt_ops.so
#########################
g++ ${CC_FLAGS} -std=c++11 -shared *.cc -o ${libpath}/${libname} \
    ${CC_INC} ${TF_LFLG} -fPIC -Wl,--rpath='$ORIGIN/..:$ORIGIN'
cd ${curdir}

echo "compile rtt_ops.so ok."