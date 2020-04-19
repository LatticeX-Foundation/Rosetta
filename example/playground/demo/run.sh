#!/bin/bash

mkdir -p log

export ROSETTA_DPASS=OFF

function judge() {
    name=$1
    script_name=./${name}.py
    log_name=./log/${name}-0.log
    message='Pass$'
    if cat ${log_name} | grep "$message" >/dev/null; then
        echo -e "\nrun ${script_name} \033[32;32;1mpass\033[0m"
        return
    else
        echo -e "\nrun ${script_name} \033[31;31;1mfail\033[0m"
        return

    fi
}

function show_case() {
    name=$1
    script_name=./${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    # chmod +x ${script_name}
    python ${script_name} --party_id=2 >log/${name}-2.log 2>&1 &
    python ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
    python ${script_name} --party_id=0 >log/${name}-0.log 2>&1
    sleep 0.5

    judge $1
}

show_case mpc_LR_with_SCE

exit 0
