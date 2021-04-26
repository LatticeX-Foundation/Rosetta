#!/bin/bash
#
# Note, this script is called by ../rosetta.sh
#
set -e

curdir=$(pwd)
ccdir=${curdir}
third_code_dir=${ccdir}/third_party
wolverine_dir=${ccdir}/third_party/emp-toolkit
builddir=${curdir}/../build
compile_options_file=${builddir}/.rtt_compile_options
if [ "${rtt_enable_128bit}" = "ON" ]; then
    builddir=${curdir}/../build128
    compile_options_file=${builddir}/.rtt_compile_options_128
fi
bindir=${builddir}/bin
mkdir -p ${bindir}
third_builddir="${builddir}/third_build"
mkdir -p ${third_builddir}

# common function
function print_compile_options() {
    echo "------------ COMPILE OPTIONS:"
    echo "                     command: ${rtt_command}"
    echo "                       phase: ${rtt_phase}"
    echo "                  build_type: ${rtt_build_type}"
    echo "enable_protocol_mpc_securenn: ${rtt_enable_protocol_mpc_securenn}"
    echo "   enable_protocol_mpc_helix: ${rtt_enable_protocol_mpc_helix}"
    echo "               enable_128bit: ${rtt_enable_128bit}"
    echo "      enable_shape_inference: ${rtt_enable_shape_inference}"
    echo "                enable_tests: ${rtt_enable_tests}"
    echo "--------------------------------------"
}
function save_compile_options() {
    rm -rf ${compile_options_file}
    touch ${compile_options_file}
    echo "${rtt_command}" >>${compile_options_file}
    echo "${rtt_phase}" >>${compile_options_file}
    echo "${rtt_build_type}" >>${compile_options_file}
    echo "${rtt_enable_protocol_mpc_securenn}" >>${compile_options_file}
    echo "${rtt_enable_protocol_mpc_helix}" >>${compile_options_file}
    echo "${rtt_enable_128bit}" >>${compile_options_file}
    echo "${rtt_enable_shape_inference}" >>${compile_options_file}
    echo "${rtt_enable_tests}" >>${compile_options_file}
}
function load_compile_options() {
    if [ ! -f "${compile_options_file}" ]; then
        echo "please compile first"
        exit 1
    fi
    x=($(cat ${compile_options_file}))
    export rtt_command=${x[0]}
    export rtt_phase=${x[1]}
    export rtt_build_type=${x[2]}
    export rtt_enable_protocol_mpc_securenn=${x[3]}
    export rtt_enable_protocol_mpc_helix=${x[4]}
    export rtt_enable_128bit=${x[5]}
    export rtt_enable_shape_inference=${x[6]}
    export rtt_enable_tests=${x[7]}
}

#
# third modules install
#

# install emp-toolkit
function install_emptoolkit() {
    # install emp-tool
    echo "to install emp-tool..."

    mkdir -p ${third_builddir}/emp-tool
    cd ${third_builddir}/emp-tool
    cmake -DCMAKE_CXX_FLAGS="-Wno-ignored-attributes -Wno-unused-but-set-variable -Wno-attributes -Wno-stringop-overflow -Wno-sign-compare" \
        ${third_code_dir}/emp-toolkit/emp-tool -DCMAKE_INSTALL_PREFIX=${builddir} -DCMAKE_PREFIX_PATH=${builddir} \
        -DCMAKE_BUILD_TYPE=${rtt_build_type} \
        -DENABLE_FLOAT=ON
    make -j && make install
    echo "install emp-tool ok."
}

# compile c++
function compile_cpp() {
    cd ${builddir}
    TF_CFLGS=""
    if [ ! -f "${builddir}/.tf_cflgs_options" ]; then
        TF_CFLGS=$(python3 -c 'import tensorflow as tf; print(tf.sysconfig.get_compile_flags()[1])')
        touch ${builddir}/.tf_cflgs_options
        echo "${TF_CFLGS}" >${builddir}/.tf_cflgs_options
    else
        TF_CFLGS=$(cat "${builddir}/.tf_cflgs_options")
    fi
    cmake ../cc ${TF_CFLGS} -DUSE_OMP=1 -DCMAKE_INSTALL_PREFIX=.install -Wno-dev \
        -DCMAKE_BUILD_TYPE=${rtt_build_type} \
        -DROSETTA_MPC_128=${rtt_enable_128bit} \
        -DROSETTA_ENABLES_SHAPE_INFERENCE=${rtt_enable_shape_inference} \
        -DROSETTA_COMPILE_TESTS=${rtt_enable_tests} \
        -DROSETTA_ENABLES_PROTOCOL_MPC_SECURENN=${rtt_enable_protocol_mpc_securenn} \
        -DROSETTA_ENABLES_PROTOCOL_MPC_HELIX=${rtt_enable_protocol_mpc_helix} \
        -DCMAKE_PREFIX_PATH=${builddir}
    make -j4
    make install
    cd ${curdir}
}

#
# normal tests
#
# common/io tests
function run_common_tests() {
    if [ -f "./common-tests" ]; then
        ./common-tests | grep -E "passed|failed"
        sleep 1
        echo "run common-tests ok."
    fi
}

function run_io_tests() {
    echo "run io-tests"
    #./mpc-io-tests-test_net_io | grep -E "passed|failed"
    #./mpc-io-tests-test_parallel_net_io | grep -E "passed|failed"

    #./mpc-io-tests-test_net_io
    echo "run io-tests ok."
    sleep 1
}

# Protocol tests
function run_protocol_mpc_test() {
    name=$1
    echo "run $name"
    killall -q $name
    if [ -f "./$name" ]; then
        ./$name >log/console-$name.log 2>&1
        sleep 0.5
    else
        echo -e "\033[31m./$name not exist!\033[0m"
    fi
}

function run_protocol_mpc_snn_tests() {
    echo "run run protocol mpc snn tests beg."
    # check
    run_protocol_mpc_test protocol_mpc_tests_snn_check

    # sepcs here
    run_protocol_mpc_test protocol_mpc_snn_tests_snn_matmul

    echo "run run protocol mpc snn tests end."
}
function run_protocol_mpc_helix_tests() {
    echo "run run protocol mpc helix tests beg."

    # check
    run_protocol_mpc_test protocol_mpc_tests_helix_check

    # sepcs here

    echo "run run protocol mpc helix tests end."
}

function run_all_modules_tests() {
    if [ $rtt_test_cpp_common -eq 1 ]; then
        cd ${bindir}
        run_common_tests
    fi

    if [ $rtt_test_cpp_netio -eq 1 ]; then
        cd ${bindir}
        run_io_tests
    fi

    if [ "$rtt_enable_protocol_mpc_securenn" == "ON" ] && [ $rtt_test_cpp_mpc_securenn -eq 1 ]; then
        cd ${bindir}
        run_protocol_mpc_snn_tests
    fi
    if [ "$rtt_enable_protocol_mpc_helix" == "ON" ] && [ $rtt_test_cpp_mpc_helix -eq 1 ]; then
        cd ${bindir}
        run_protocol_mpc_helix_tests
    fi

}

#
# performance
#
function run_protocol_mpc_snn_perfs() {
    echo "run run protocol mpc snn performance beg."
    run_protocol_mpc_test protocol_mpc_tests_snn_perf

    echo "run run protocol mpc snn performance end."
}

function run_protocol_mpc_helix_perfs() {
    echo "run run protocol mpc helix performance beg."
    run_protocol_mpc_test protocol_mpc_tests_helix_perf

    echo "run run protocol mpc helix performance end."
}

function run_all_modules_perfs() {
    if [ "$rtt_enable_protocol_mpc_securenn" == "ON" ] && [ $rtt_perf_cpp_mpc_securenn -eq 1 ]; then
        cd ${bindir}
        run_protocol_mpc_snn_perfs
    fi
    if [ "$rtt_enable_protocol_mpc_helix" == "ON" ] && [ $rtt_perf_cpp_mpc_helix -eq 1 ]; then
        cd ${bindir}
        run_protocol_mpc_helix_perfs
    fi
}

if [ "${rtt_command}" = "compile" ]; then
    print_compile_options

    # install deps. emp-toolkit, ...
    install_emptoolkit

    compile_cpp
    save_compile_options
elif [ "${rtt_command}" = "test" ] || [ "${rtt_command}" = "perf" ]; then
    _rtt_command=${rtt_command}
    load_compile_options
    #print_compile_options
    if [ "${rtt_enable_tests}" != "ON" ]; then
        echo "Please set --enable-tests when compiling."
        exit 1
    fi

    cd ${bindir}
    cp -f ${ccdir}/conf/CONFIG*.json ./
    cp -rf ${ccdir}/certs ./
    mkdir -p log out key data
    cd ${curdir}

    if [ "${_rtt_command}" == "test" ]; then
        run_all_modules_tests
    else
        run_all_modules_perfs
    fi
else
    echo "Unknow command"
fi

cd ${curdir}
