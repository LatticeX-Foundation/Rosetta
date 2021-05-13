#!/bin/bash

set -x

curdir=$(pwd)
builddir=${curdir}/build
bindir=${builddir}/bin
mkdir -p ${bindir}

# compile
cd ${builddir}
#cmake .. -DCMAKE_INSTALL_PREFIX=.install && make -j8 all && make install
cmake .. -DCMAKE_INSTALL_PREFIX=.install -DCMAKE_BUILD_TYPE=Debug && make -j8 all && make install

# certs
cd ${curdir}
cp -r certs ${bindir}/

# test
cd ${bindir}
echo "run mpc-io-tests"
./mpc-io-tests-test_net_io | grep -E "passed|failed"
./mpc-io-tests-test_parallel_net_io | grep -E "passed|failed"

# Done!
cd ${curdir}

exit 0
