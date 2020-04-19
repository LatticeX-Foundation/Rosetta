#!/bin/bash

# find . -name '*.sh' | xargs chmod 755
# find . -name '*.py' | xargs chmod 755

# for debug
# set -x

# This script for compiling all c++ projects and python-setup project
# Also runing all tests

curdir=$(pwd)
ccdir=${curdir}/cc/
mpcopdir=${curdir}/cc/tf/mpcops/
dynamic_pass=${curdir}/cc/tf/dpass/
cctf_misc=${curdir}/cc/tf/misc/


# run a compile-and-run stage
function run_stage_1() {
    echo "stage 1: compile and test all c++ basic libraries, io, op, etc."
    cd ${ccdir}
    bash ./compile_and_test.sh
    cd ${curdir}
    echo "run stage 1 ok."
}

function run_stage_2() {
    echo "stage 2: compile MPC operations for tensorflow"
    cd ${mpcopdir}
    bash ./compile_and_test.sh
    cd ${curdir}
    echo "run stage 2 ok."
}

function run_stage_3() {
    echo "stage 3: compile dynamic pass for tensorflow"
    cd ${dynamic_pass}
    bash ./compile_and_test.sh
    cd ${curdir}
    echo "run stage 3 ok."
}

function run_stage_4() {
    echo "stage 4: build and install .whl for python"
    cd ${curdir}
    python setup.py build_ext
    python setup.py bdist_wheel

    pv=$(python -c 'import sys; print(sys.version_info[0])')
    pip_cmd=pip
    if [ $pv == '3' ]; then
        pip_cmd=pip3
    fi

    if [ $USER == "root" ]; then
        ${pip_cmd} install dist/*.whl
    else
        #python setup.py build_ext install --prefix=~/.local
        ${pip_cmd} install dist/*.whl --user
    fi

    cd ${curdir}
    echo "run stage 4 ok."
}

# run all the stages
function run_all() {
    run_stage_1
    run_stage_2
    run_stage_3
    run_stage_4

    echo "run all the stages ok."
}

# get choice, default choice means run all the stages
stage_choice=0
if [ $# -ge 1 ]; then
    stage_choice=$1
    echo "run stage: ${stage_choice}..."
else
    echo "run all the stages..."
fi

# main
function run_main() {
    echo "start to run..."
    case "${stage_choice}" in
    0)
        echo "run stage 1, 2, 3, 4 one by one..."
        run_all
        ;;
    1)
        echo "choice is stage 1, sml compilation or test."
        run_stage_1
        ;;
    2)
        echo "choice is stage 2, compile mpcop (c++)."
        run_stage_2
        ;;
    3)
        echo "choice is stage 3, compiler dynamic pass (c++)."
        run_stage_3
        ;;
    4)
        echo "choice is stage 4, compile setup.py."
        run_stage_4
        ;;
    *)
        echo "bad choice of stage!!!"
        exit 1
        ;;
    esac

    echo "ending."
}

run_main
