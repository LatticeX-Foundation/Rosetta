#!/bin/bash

target="modules"
curdir=$(pwd)

ccdir=${curdir}

BUILD_TYPE=Release
builddir=${curdir}/../build

if [[ $ROSETTA_MPC_128 ]] && [[ $ROSETTA_MPC_128 == "ON" ]]; then
    builddir=${curdir}/../build128
else
    ROSETTA_MPC_128=OFF
fi

bindir=${builddir}/bin
mkdir -p ${bindir}

# compile c++
cd ${builddir}
TF_CFLGS=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_compile_flags()[1])')
#TF_CFLAGS=$(python3 -c "import tensorflow as tf; print(' '.join(tf.sysconfig.get_compile_flags()), end='')")
if [ $# -ge 2 ]; then
    if [ $1 == "Debug" ]; then
        BUILD_TYPE=Debug
    fi

    if [ $2 == "128" ]; then
        MPC_128=ON
    fi

    # if you want to compile all cpp examples & tests, please set -DROSETTA_COMPILE_TESTS=ON
    cmake ../cc ${TF_CFLGS} -DUSE_OMP=1 -DCMAKE_INSTALL_PREFIX=.install -Wno-dev \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DROSETTA_MPC_128=${MPC_128} -DROSETTA_COMPILE_TESTS=ON
else
    echo "compile $1 ..."
    if [ "$1" == "modules" ]; then
        cd modules && make -j8 && make install
    elif [ "$1" == "tf" ]; then
        cd tf && make -j8 && make install
    else
        echo "bad target: $1"
        exit 1
    fi
fi

# prepare
cd ${bindir}
cp -f ${ccdir}/conf/CONFIG*.json ./
cp -rf ${ccdir}/certs ./
mkdir -p log out key

function run_common_tests() {
    if [ -f "./common-tests" ]; then
        ./common-tests | grep -E "passed|failed"
        sleep 1
        echo "run common-tests ok."
    fi
}

function run_mpc_io_tests() {
    echo "run mpc-io-tests"
    #./mpc-io-tests-test_net_io | grep -E "passed|failed"
    #./mpc-io-tests-test_parallel_net_io | grep -E "passed|failed"

    echo "run mpc-io-tests ok."
    sleep 1
}

function test_protocol() {
    name=$1
    echo "run $name"
    killall -q $name
    if [ -f "./$name" ]; then
        ./$name
        sleep 1
    else
        echo -e "\033[31m./$name not exist!\033[0m"
    fi
}

function run_protocol_mpc_snn() {
    # echo "run protocol_mpc_SNN"
    test_protocol protocol_mpc_snn_tests_snn_check
    #test_protocol protocol_mpc_snn_tests_snn_perf
    echo "run_protocol_mpc_snn ok."
}

# run all tests (including common, io, op, ..)
function run_all_modules_tests() {
    # run all tests (including common, io, op, ..)
    echo "rum common tests..."
    run_common_tests

    echo "run mpc-io tests..."
    run_mpc_io_tests

    echo "run protocol mpc-SNN test.."
    run_protocol_mpc_snn
}

# run tests
if [ $# -ge 1 ]; then
    echo "run tests of cc ... "
    if [ "$1" == "modules" ]; then
        run_all_modules_tests
    elif [ $1 == "tf" ]; then
        echo "run tensorflow test... now [nothing]."
    else
        echo "bad target: $1"
        exit 1
    fi
fi

cd ${curdir}
