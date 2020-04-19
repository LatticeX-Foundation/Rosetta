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

# Done!
cd ${curdir}

exit 0
