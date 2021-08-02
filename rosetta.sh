#!/bin/bash
#
# Notes:
#
# In order to use autocomplete, you may need to install 'bash-completion'.
#
# Since our scripts are not installed into the system path,
# and the execution path is different for each user.
# If you want the completions to take effect on the current terminal,
# execute the following command once:
#   source ./rtt_completion
#
# Important! If you want to run tests/perf under 128bit(only mpc), please export ROSETTA_MPC_128=ON first.
#

. ./rosetta_.sh

curdir=$(pwd)

version="1.0.0"
# usage
E="./"$(basename $0)
function show_usage() {
    echo "Welcome to Rosetta @ LatticeX Foundation. (https://github.com/LatticeX-Foundation/Rosetta)"
    echo ""
    echo "USAGE:"
    echo "  $E COMMAND [OPTION]"
    echo ""
    echo "COMMANDS:"
    echo "  compile    compile the libraries, protocol libraries, secure-ops, python-export, etc."
    echo "  install    install .whl"
    echo "  test       run normal test cases"
    echo "  perf       run performance test cases (now, only protocol-level[c++])"
    echo "  clean      clean workspace"
    echo ""
    echo "MORE DETAILS:"
    echo "  $E COMMAND --help"
    echo ""
    # #################################
    echo -e "# ${PINK}Notes:${NC}"
    echo "#"
    echo "# In order to use autocomplete, you may need to install 'bash-completion'."
    echo "#"
    echo "# Since our scripts are not installed into the system path,"
    echo "# and the execution path is different for each user."
    echo "# If you want the completions to take effect on the current terminal,"
    echo "# execute the following command once:"
    echo "#   source ./rtt_completion"
    echo "#"
    echo "# Important! If you want to run tests/perf under 128bit(only mpc), please export ROSETTA_MPC_128=ON first."
    echo ""
}

function show_compile_usage() {
    echo "USAGE:"
    echo "  $E compile [OPTION]"
    echo "  e.g.: $E compile --enable-protocol-mpc-securenn"
    echo ""
    echo "OPTIONS:"
    echo "  -h --help                               Show this usage"
    echo "     --phase name                         [all] One of [modules,py,all]"
    echo "     --build-type                         [Release] One of [Release,Debug]"
    echo ""
    echo "  There are some options for 'phase' of modules or all (if have supported by Rosetta):"
    echo "     --enable-gmssl                       [OFF] Enable GmSSL(NOT SUPPORTED NOW), default is OpenSSL"
    echo "     --enable-all                         [OFF] Enable all the following options"
    echo "       --enable-protocol-mpc-securenn     [OFF] Secure Multi-party Computation (base on SecureNN)"
    echo "       --enable-protocol-mpc-helix        [OFF] Secure Multi-party Computation (base on Helix)"
    echo "       --enable-128bit                    [OFF] 128-bit data type"
    echo "       --enable-tests                     [OFF] Compile all the test cases"
    echo ""
    echo "  The default options: --phase all --build-type Release"
    echo ""
}

function show_test_usage() {
    echo "USAGE:"
    echo "  $E test <OPTION|NAME...>"
    echo "  e.g.: $E test common netio mpc op"
    echo ""
    echo "OPTIONS:"
    echo "  -h --help           Show this usage"
    echo ""
    echo "NAMES:"
    echo "     all              Including all the following NAMES (If and only if have supported by Rosetta)"
    echo ""
    echo "  For modules(c++):"
    echo "     common           Common library"
    echo "     netio            Network IO"
    echo "     mpc              Protocol MPC (including mpc-*)"
    echo "     mpc-securenn     Protocol MPC (SecureNN)"
    echo "     mpc-helix        Protocol MPC (Helix)"
    echo ""
    echo "  For python:"
    echo "     op               Operators"
    echo "     gradop           Gradient operators"
    echo "     spass            Static pass"
    echo "     dpass            Dynamic pass"
    echo "   multi-task         multi-task"
    echo "   single-task         single-task"
    echo "     other            Other tests"
    echo ""
    echo "NOTE:"
    echo "  If you want to run under 128bit data type (only mpc), please export ROSETTA_MPC_128=ON first."
    echo ""
}

function show_perf_usage() {
    echo "USAGE:"
    echo "  $E perf <OPTION|NAME...>"
    echo "  e.g.: $E perf mpc"
    echo ""
    echo "OPTIONS:"
    echo "  -h --help           Show this usage"
    echo ""
    echo "NAMES:"
    echo "     all              All the following NAMES (if have supported by Rosetta)"
    echo ""
    echo "  For modules(c++):"
    echo "     mpc              Protocol MPC (including mpc-*)"
    echo "     mpc-securenn     Protocol MPC (SecureNN)"
    echo "     mpc-helix        Protocol MPC (Helix)"
    echo ""
    echo "NOTE:"
    echo "If you want to run under 128bit data type (only mpc), please export ROSETTA_MPC_128=ON first"
    echo ""
}

function show_clean_usage() {
    echo "USAGE:"
    echo "  $E clean"
    echo ""
}
function show_install_usage() {
    echo "USAGE:"
    echo "  $E install"
    echo ""
}

if [ $# -lt 1 ]; then
    show_usage
    exit 1
fi

#
#
# parse arguments

cmd=${1}
if [ "${cmd}" = "compile" ]; then
    # default
    phase=all
    build_type=Release
    enable_gmssl=OFF
    enable_all=0
    enable_protocol_mpc_securenn=OFF
    enable_protocol_mpc_helix=OFF
    enable_128bit=OFF
    enable_shape_inference=OFF
    enable_tests=OFF

    # if the version of python is greater than 3.6, shape inference is enabled by default
    python_version=$(python3 -c 'import sys;ver=sys.version_info;print(str(ver[0])+"."+str(ver[1]))')
    python_major_version=$(echo ${python_version} | cut -d. -f1)
    python_minor_version=$(echo ${python_version} | cut -d. -f2)
    if [ "${python_major_version}" -eq 3 ] && [ "${python_minor_version}" -gt 6 ]; then
        enable_shape_inference=ON
    fi

    ARGS=$(getopt -o "h" -l "help,phase:,build-type:,enable-gmssl,enable-all,enable-protocol-mpc-securenn,enable-protocol-mpc-helix,enable-128bit,enable-tests" -n "$0" -- "$@")
    eval set -- "${ARGS}"
    while true; do
        case "${1}" in
        -h | --help)
            show_compile_usage
            exit 0
            shift
            ;;
        --phase)
            phase=${2}
            shift 2
            ;;
        --build-type)
            build_type=${2}
            shift 2
            ;;
        --enable-gmssl)
            enable_gmssl=ON
            shift
            ;;
        --enable-all)
            enable_all=1
            shift
            ;;
        --enable-protocol-mpc-securenn)
            enable_protocol_mpc_securenn=ON
            shift
            ;;
        --enable-protocol-mpc-helix)
            enable_protocol_mpc_helix=ON
            shift
            ;;
        --enable-128bit)
            enable_128bit=ON
            shift
            ;;
        --enable-tests)
            enable_tests=ON
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            shift
            break
            ;;
        esac
    done
    if [ ${enable_all} -eq 1 ]; then
        enable_protocol_mpc_securenn=ON
        enable_protocol_mpc_helix=ON
        enable_128bit=ON
        enable_tests=ON
    fi
    # valid check [todo]

    # show meta info
    echo -e "${BLUE}Ready to compile rosetta${NC}"

    # run compile
    export rtt_command=compile
    export rtt_phase=${phase}
    export rtt_build_type=${build_type}
    # [ujnss] in general version, not supported GMSSL
    #export rtt_enable_gmssl=${enable_gmssl}
    export rtt_enable_gmssl=OFF
    export rtt_enable_protocol_mpc_securenn=${enable_protocol_mpc_securenn}
    export rtt_enable_protocol_mpc_helix=${enable_protocol_mpc_helix}
    export rtt_enable_shape_inference=${enable_shape_inference}
    export rtt_enable_tests=${enable_tests}
    if [ "${rtt_phase}" = "all" ] || [ "${rtt_phase}" = "modules" ]; then
        if [ "${enable_128bit}" = "ON" ]; then
            # compile 128bit first
            export rtt_enable_128bit=ON
            run_rosetta_compile_modules
        fi
        export rtt_enable_128bit=OFF
        run_rosetta_compile_modules
    fi
    if [ "${rtt_phase}" = "all" ] || [ "${rtt_phase}" = "py" ]; then
        run_rosetta_compile_python
    fi
    exit 0
elif [ "${cmd}" = "test" ]; then
    ARGS=$(getopt -o "h" -l "help" -n "$0" -- "$@")
    eval set -- "${ARGS}"
    while true; do
        case "${1}" in
        -h | --help)
            show_test_usage
            exit 0
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            shift
            break
            ;;
        esac
    done

    #default
    test_all=0
    test_cpp_common=0
    test_cpp_netio=0
    test_cpp_mpc=0
    test_cpp_mpc_securenn=0
    test_cpp_mpc_helix=0
    test_py_op=0
    test_py_gradop=0
    test_py_spass=0
    test_py_dpass=0
    test_py_multi_task=0
    test_py_single_task=0
    test_py_other=0

    for arg in $*; do
        case $arg in
        all) test_all=1 ;;
        # cpp
        common) test_cpp_common=1 ;;
        netio) test_cpp_netio=1 ;;
        mpc) test_cpp_mpc=1 ;;
        mpc-securenn) test_cpp_mpc_securenn=1 ;;
        mpc-helix) test_cpp_mpc_helix=1 ;;

        # python
        op) test_py_op=1 ;;
        gradop) test_py_gradop=1 ;;
        spass) test_py_spass=1 ;;
        dpass) test_py_dpass=1 ;;
        multi-task) test_py_multi_task=1 ;;
        single-task) test_py_single_task=1 ;;
        other) test_py_other=1 ;;
        esac
    done
    if [ $test_all -eq 1 ]; then
        test_cpp_common=1
        test_cpp_mpc=1
        test_cpp_mpc_securenn=1
        test_cpp_mpc_helix=1
        test_py_op=1
        test_py_gradop=1
        test_py_spass=1
        test_py_dpass=1
        test_py_multi_task=1
        test_py_single_task=1
        test_py_other=1
    fi

    export rtt_command=test
    export rtt_test_cpp_common=${test_cpp_common}
    export rtt_test_cpp_mpc=${test_cpp_mpc}
    export rtt_test_cpp_mpc_securenn=${test_cpp_mpc_securenn}
    export rtt_test_cpp_mpc_helix=${test_cpp_mpc_helix}
    export rtt_test_py_op=${test_py_op}
    export rtt_test_py_gradop=${test_py_gradop}
    export rtt_test_py_spass=${test_py_spass}
    export rtt_test_py_dpass=${test_py_dpass}
    export rtt_test_multi_task=${test_py_multi_task}
    export rtt_test_single_task=${test_py_single_task}
    export rtt_test_py_other=${test_py_other}
    echo "      test_cpp_common: ${rtt_test_cpp_common}"
    echo "         test_cpp_mpc: ${rtt_test_cpp_mpc}"
    echo "test_cpp_mpc_securenn: ${rtt_test_cpp_mpc_securenn}"
    echo "   test_cpp_mpc_helix: ${rtt_test_cpp_mpc_helix}"
    echo "           test_py_op: ${rtt_test_py_op}"
    echo "       test_py_gradop: ${rtt_test_py_gradop}"
    echo "        test_py_spass: ${rtt_test_py_spass}"
    echo "        test_py_dpass: ${rtt_test_py_dpass}"
    echo "   test_py_multi_task: ${test_py_multi_task}"
    echo "   test_py_single_task: ${test_py_single_task}"
    echo "        test_py_other: ${rtt_test_py_other}"

    echo -e "${BLUE}Ready to run test${NC}"
    run_rosetta_test_modules
    run_rosetta_test_python
    exit 0
elif [ "${cmd}" = "perf" ]; then
    ARGS=$(getopt -o "h" -l "help" -n "$0" -- "$@")
    eval set -- "${ARGS}"
    while true; do
        case "${1}" in
        -h | --help)
            show_perf_usage
            exit 0
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            shift
            break
            ;;
        esac
    done

    #default
    perf_all=0
    perf_cpp_mpc=0
    perf_cpp_mpc_securenn=0
    perf_cpp_mpc_helix=0

    for arg in $*; do
        case $arg in
        all) test_all=1 ;;
        # cpp
        mpc) perf_cpp_mpc=1 ;;
        mpc-securenn) perf_cpp_mpc_securenn=1 ;;
        mpc-helix) perf_cpp_mpc_helix=1 ;;
        esac
    done
    if [ $test_all -eq 1 ]; then
        perf_cpp_mpc=1
        perf_cpp_mpc_securenn=1
        perf_cpp_mpc_helix=1
    fi

    export rtt_command=perf
    export rtt_perf_cpp_mpc=${perf_cpp_mpc}
    export rtt_perf_cpp_mpc_securenn=${perf_cpp_mpc_securenn}
    export rtt_perf_cpp_mpc_helix=${perf_cpp_mpc_helix}
    echo "         perf_cpp_mpc: ${rtt_perf_cpp_mpc}"
    echo "perf_cpp_mpc_securenn: ${rtt_perf_cpp_mpc_securenn}"
    echo "   perf_cpp_mpc_helix: ${rtt_perf_cpp_mpc_helix}"

    echo -e "${BLUE}Ready to run perf${NC}"
    run_rosetta_perf_modules
    exit 0
elif [ "${cmd}" = "clean" ]; then
    ARGS=$(getopt -o "h" -l "help" -n "$0" -- "$@")
    eval set -- "${ARGS}"
    while true; do
        case "${1}" in
        -h | --help)
            show_clean_usage
            exit 0
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            shift
            break
            ;;
        esac
    done
    echo -e "${BLUE}Ready to clean${NC}"
    run_rosetta_clean
    exit 0
elif [ "${cmd}" = "install" ]; then
    ARGS=$(getopt -o "h" -l "help" -n "$0" -- "$@")
    eval set -- "${ARGS}"
    while true; do
        case "${1}" in
        -h | --help)
            show_install_usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            shift
            break
            ;;
        esac
    done
    echo -e "${BLUE}Ready to install${NC}"
    run_rosetta_install_python
    exit 0
else
    ARGS=$(getopt -o "hv" -l "help,version" -n "$0" -- "$@")
    eval set -- "${ARGS}"
    while true; do
        case "${1}" in
        -h | --help)
            show_usage
            exit 0
            shift
            ;;
        -v | --version)
            echo "version ${version}"
            exit 0
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            shift
            break
            ;;
        esac
    done
    echo -e "${RED}Unsupported command${NC}"
    show_usage
    exit 1
fi

echo "never enter here"
