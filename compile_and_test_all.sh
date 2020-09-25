#!/bin/bash
# find . -name '*.sh' | xargs chmod 755
# find . -name '*.py' | xargs chmod 755


# This script for compiling all c++ projects and python-setup project
# Also runing all tests

curdir=$(pwd)
ccdir=${curdir}/cc/

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# run a compile-and-run stage
function run_stage_prepare() {
    echo -e "stage 0: cmake build and dependencies install."
    cd ${ccdir}
    bash ./compile_and_test.sh $*
    cd ${curdir}
    echo -e "${GREEN}run stage prepare ok.${NC}"
}

function run_stage_1() {
    echo -e "stage 1: compile and test core c++ libraries, io, op, protocol etc."
    cd ${ccdir}
    bash ./compile_and_test.sh modules
    cd ${curdir}
    echo -e "${GREEN}run stage 1 ok.${NC}"
}

function run_stage_2() {
    echo -e "stage 2: compile MPC libraries for tensorflow"
    cd ${ccdir}
    bash ./compile_and_test.sh tf

    cd ${curdir}
    echo -e "${GREEN}run stage 2 ok.${NC}"
}

function run_stage_3() {
    echo -e "stage 3: build and install .whl for python"
    cd ${curdir}

    python3 setup.py build_ext
    python3 setup.py bdist_wheel

    pv=$(python3 -c 'import sys; print(sys.version_info[0])')
    pip_cmd=pip
    if [ $pv == '3' ]; then
        pip_cmd=pip3
    fi

    if [ $USER == "root" ]; then
        ${pip_cmd} install dist/*.whl
    else
        ${pip_cmd} install dist/*.whl --user
    fi

    cd ${curdir}
    echo -e "${GREEN}run stage 3 ok.${NC}"
}

# run all the stages
function run_all() {
    unset ROSETTA_MPC_128
    use_128=64

    run_stage_prepare ${build_type} ${use_128}
    run_stage_1
    run_stage_2

    echo -e "run 64bits binary all ok."
}

# run all the stages
function run_all_with_128() {
    if [ ${use_128} == "128" ]; then
        echo -e "run and compile 128 bits binaries..."
    else
        echo -e "please specify 128 bits to compile and run !!!"
        exit 1
    fi
    # run and compile 128 bits
    export ROSETTA_MPC_128=ON
    run_stage_prepare ${build_type} ${use_128}
    run_stage_1
    run_stage_2

    echo -e "run and compile 128 bits binaries ok."

    # build 128 _rosetta.xxxx.so and backup
    python3 setup.py build_ext
    unset ROSETTA_MPC_128

    # run and compile 64 bits binaries and .whl package
    use_128=64
    run_stage_prepare ${build_type} ${use_128}
    run_stage_1
    run_stage_2

    run_stage_3
    echo -e "run and compile 64 bits binaries ok."

    echo -e "run 128-bits, 64-bits binaries all ok."
}

# get choice, default choice means run all the stages
stage_choice=x
# build_type, default is Release
build_type=Release
# use 128 bits mpc type
use_128=64
if [ $# -ge 1 ]; then
    stage_choice=$1
    echo -e "run stage: ${stage_choice}..."
else
    echo -e "run all the stages..."
fi

if [ $# -ge 2 ]; then
    build_type=$2
    echo -e "build type: ${build_type}..."
fi

if [ $# -ge 3 ]; then
    use_128=$3
    echo -e "use 128 bits mpc: ${use_128}..."
fi

# main
function run_main() {
    echo -e "start to run..."
    case "${stage_choice}" in
    x)
        echo -e "run stage 1, 2, 3, 4 one by one..."
        run_all
        ;;
    0)
        echo -e "run stage prepare, cmake build..."
        run_stage_prepare
        ;;
    1)
        echo -e "choice is stage 1, sml compilation or test."
        run_stage_1
        ;;
    2)
        echo -e "choice is stage 2, tensorflow libraries compilation or test."
        run_stage_2
        ;;
    3)
        echo -e "choice is stage 4, compile setup.py."
        run_stage_3
        ;;
    4)
        echo -e "choice for all with 128 bits binaries, test mpcop (python)."
        use_128=128 # force 128 bits
        run_all_with_128
        ;;
    *)
        echo -e "bad choice of stage!!!"
        exit 1
        ;;
    esac

    echo -e "ending."
}

run_main
