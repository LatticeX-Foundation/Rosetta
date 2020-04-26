#!/bin/bash

# find . -name '*.sh' | xargs chmod 755
# find . -name '*.py' | xargs chmod 755

curdir=$(pwd)

ccdir=${curdir}
builddir=${curdir}/../build/
bindir=${builddir}/bin
mkdir -p ${bindir}

# compile c++
cd ${builddir}
TF_CFLGS=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_compile_flags()[1])')
cmake ../cc/ ${TF_CFLGS} -DUSE_OMP=1 -DCMAKE_INSTALL_PREFIX=.install && make -j8 all && make install
#cmake ../cc/ ${TF_CFLGS} -DUSE_OMP=1 -DCMAKE_INSTALL_PREFIX=.install -DCMAKE_BUILD_TYPE=Debug && make -j8 all && make install

# prepare
cd ${bindir}
cp -f ${ccdir}/conf/CONFIG*.json ./
cp -rf ${ccdir}/certs ./
mkdir -p log out key

echo "run protocol_mpc_SNN"
killall -q protocol_mpc_SNN
./protocol_mpc_SNN 2 CONFIG.json >log/protocol-mpc-snn-log2.txt 2>&1 &
./protocol_mpc_SNN 1 CONFIG.json >log/protocol-mpc-snn-log1.txt 2>&1 &
./protocol_mpc_SNN 0 CONFIG.json >log/protocol-mpc-snn-log0.txt 2>&1

grep initialize_communication log/protocol-mpc-snn-log0.txt

cd ${curdir}
