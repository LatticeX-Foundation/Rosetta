#!/bin/bash
# find . -name '*.sh' | xargs chmod 755
# find . -name '*.py' | xargs chmod 755

# for debug
set -x
bash ./kill.sh
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
    bash ./compile_and_test.sh
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
    echo -e "stage 3: compile dynamic pass for tensorflow, [run stage2] !"
    return

    cd ${dynamic_pass}
    bash ./compile_and_test.sh
    echo -e "${GREEN}compile dynamic pass ok.${NC}"

    cd ${curdir}
    echo -e "${GREEN}run stage 3 ok.${NC}"
}

function run_stage_4() {
    echo -e "stage 4: build and install .whl for python"
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
    echo -e "${GREEN}run stage 4 ok.${NC}"
}

# run all the stages
function run_all() {
    run_stage_prepare
    run_stage_1
    run_stage_2
    run_stage_3
    run_stage_4

    echo -e "run all the stages ok."
}

# get choice, default choice means run all the stages
stage_choice=x
if [ $# -ge 1 ]; then
    stage_choice=$1
    echo -e "run stage: ${stage_choice}..."
else
    echo -e "run all the stages..."
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
    4)
        echo -e "choice is stage 4, compile setup.py."
        run_stage_4
        ;;
    *)
        echo -e "bad choice of stage!!!"
        exit 1
        ;;
    esac

    echo -e "ending."
}

run_main
