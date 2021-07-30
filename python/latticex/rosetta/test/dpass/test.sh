#!/bin/bash

mkdir -p log


function judge() {
    name=$1
    script_name=./${name}.py
    log_name=./log/log-${name}-0.txt
    message='^Pass$'
    if cat ${log_name} | grep "$message" >/dev/null
    then
        echo -e "\nrun ${script_name} \033[32;32;1mpass\033[0m"
        return
    else
        echo -e "\nrun ${script_name} \033[31;31;1mfail\033[0m"
        return

    fi
}

function test_dpass() {
    name=$1
    script_name=./${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    # chmod +x ${script_name}
    python3 ${script_name} --party_id=2 >log/log-${name}-2.txt 2>&1 &
    python3 ${script_name} --party_id=1 >log/log-${name}-1.txt 2>&1 &
    python3 ${script_name} --party_id=0 >log/log-${name}-0.txt 2>&1

    wait
    # sleep 0.5

    judge $1
}

test_dpass test_mpc_optapplyX_pass_1
test_dpass test_mpc_optapplyX_pass_2
test_dpass test_mpc_optapplyX_pass_3
test_dpass test_mpc_optapplyX_pass_4
test_dpass test_mpc_savev2_pass_1
test_dpass test_mpc_savev2_pass_2
test_dpass test_mpc_savev2_pass_3
test_dpass test_mpc_savev2_pass_4
test_dpass test_mpc_mixed_pass_1
test_dpass test_mpc_mixed_pass_2
test_dpass test_mpc_mixed_pass_3
test_dpass test_mpc_mixed_pass_4

exit 0
