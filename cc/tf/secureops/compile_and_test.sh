#!/bin/bash

set -x

curdir=$(pwd)
ccdir=${curdir}/../../
builddir=${ccdir}/../build/
libpath=${builddir}/lib
if [ ! -e ${libpath} ]; then
    mkdir -p ${libpath}
fi


echo "compiling secure_ops.so..."

#
## compile rrt_op.so
#

cd ${curdir}
CFLAGS=" -Wno-format-overflow -g "

PROJECT_BASE=${ccdir}../
libname="libsecure_ops.so"

rm -f ${libpath}/${libname}

TF_INC=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_include())')
TF_LIB=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_lib())')
TF_CFLG=$(python3 -c 'import tensorflow as tf; print(" ".join(tf.sysconfig.get_compile_flags()))')
TF_LFLG=$(python3 -c 'import tensorflow as tf; print(" ".join(tf.sysconfig.get_link_flags()))')

CC_INC="-I${TF_INC} -I${PROJECT_BASE}/cc/third_party/rapidjson/include -I${PROJECT_BASE} -I${ccdir}tf/mpcops/ops/thirdparty/"
CC_FLAGS="${CFLAGS} ${TF_CFLG} -std=c++11 -g"
LD_FLASG="-L ${builddir}/lib -lmpc-snn -lprotocol-base -lprotocol-api ${TF_LFLG}"
#########################
## compile secure_ops.so
#########################
g++ ${CC_FLAGS} -shared *.cc \
    ${CC_INC} ${LD_FLASG} -fPIC -Wl,--rpath='$ORIGIN/..:$ORIGIN' -o ${libpath}/${libname}
cd ${curdir}

echo "compile secure_ops.so ok."

# run test
# echo "run secure_snn ops..."
# #bash test.sh
# echo "run secure_snn ops ok."
