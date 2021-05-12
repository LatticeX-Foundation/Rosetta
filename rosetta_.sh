#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
PINK='\033[0;35m'
NC='\033[0m' # No Color

# global variables
curdir=$(pwd)
ccdir=${curdir}/cc/
snn_mpcop_dir=${curdir}/cc/tf/mpcops/
cctf_misc=${curdir}/cc/tf/misc/
mpcop_py_testdir=${curdir}/python/latticex/rosetta/secure/decorator/test_cases
mpcgradop_py_testdir=${curdir}/python/latticex/rosetta/secure/grads_ops/test_cases
spass_py_testdir=${curdir}/python/latticex/rosetta/secure/spass/test_cases
misc_py_testdir=${cctf_misc}/test_cases

pv=$(python3 -c 'import sys; print(sys.version_info[0])')
pip_cmd=pip
if [ "$pv" == '3' ]; then
  pip_cmd=pip3
fi

# return 1 if w in arr; else 0
function exist_in_arr() {
  local w=$1
  local arr=$2
  echo "$arr" | grep -w "$w" &>/dev/null && echo 1 && return
  echo 0
}

function run_rosetta_compile_modules() {
  echo -e "stage run_rosetta_compile_modules."
  cd ${ccdir}
  bash ./compile_and_test_inner.sh
  cd ${curdir}
  echo -e "${GREEN}run stage run_rosetta_compile_modules ok.${NC}"
}

function run_rosetta_compile_python() {
  echo -e "stage run_rosetta_compile_python."
  cd ${curdir}
  if [ "$USER" == "root" ]; then
    ${pip_cmd} uninstall latticex_rosetta -y
  else
    python3 -m pip uninstall latticex-rosetta -y # for the current user
  fi

  python3 setup.py build_ext
  python3 setup.py bdist_wheel
  cd ${curdir}
  echo -e "${GREEN}run stage run_rosetta_compile_python ok.${NC}"
}

function run_rosetta_install_python() {
  echo -e "stage run_rosetta_install_python."
  cd ${curdir}

  if [ "$USER" == "root" ]; then
    ${pip_cmd} install dist/*.whl
  else
    ${pip_cmd} install dist/*.whl --user
  fi

  cd ${curdir}
  echo -e "${GREEN}run_rosetta_install_python.${NC}"
}

function run_rosetta_test_modules() {
  echo -e "stage run_rosetta_test_modules."
  cd ${ccdir}
  bash ./compile_and_test_inner.sh
  cd ${curdir}
  echo -e "${GREEN}run stage run_rosetta_test_modules ok.${NC}"
}

function run_rosetta_perf_modules() {
  echo -e "stage run_rosetta_perf_modules."
  cd ${ccdir}
  bash ./compile_and_test_inner.sh
  cd ${curdir}
  echo -e "${GREEN}run stage run_rosetta_perf_modules ok.${NC}"
}

function run_rosetta_test_python() {
  echo -e "stage run_rosetta_test_python."

  if [ $rtt_test_py_op -eq 1 ]; then
    mkdir -p ${mpcop_py_testdir}/certs
    cp -f ${ccdir}/certs/* ${mpcop_py_testdir}/certs/
    cd ${mpcop_py_testdir}
    echo -e "run mpcop test cases..."
    if [ -f ./test.sh ]; then
      bash ./test.sh
    fi
  fi

  if [ $rtt_test_py_gradop -eq 1 ]; then
    mkdir -p ${mpcgradop_py_testdir}/certs
    cp -f ${ccdir}/certs/* ${mpcgradop_py_testdir}/certs/
    cd ${mpcgradop_py_testdir}
    echo -e "run grad mpcop test cases..."
    if [ -f ./test.sh ]; then
      bash ./test.sh
    fi
  fi

  if [ $rtt_test_py_spass -eq 1 ]; then
    mkdir -p ${spass_py_testdir}/certs
    cp -f ${ccdir}/certs/* ${spass_py_testdir}/certs/
    cd ${spass_py_testdir}
    echo -e "run static pass test cases"
    if [ -f ./test.sh ]; then
      bash ./test.sh
    fi
  fi

  if [ $rtt_test_py_other -eq 1 ]; then
    mkdir -p ${misc_py_testdir}/certs
    cp -f ${ccdir}/certs/* ${misc_py_testdir}/certs/
    cd ${misc_py_testdir}
    echo -e "run misc test cases"
    if [ -f ./test.sh ]; then
      bash ./test.sh
    fi
  fi

  cd ${curdir}
  echo -e "${GREEN}run_rosetta_test_python.${NC}"
}

function run_rosetta_clean() {
  echo -e "stage run_rosetta_clean."
  cd ${curdir}
  ./clean_directory.sh
  cd ${curdir}
  ./clean_pip.sh
  echo -e "${GREEN}run_rosetta_clean.${NC}"
}
