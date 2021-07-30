#!/bin/bash

mkdir -p log
# disable dynamic pass
export ROSETTA_DPASS=OFF

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

function test_op() {
    name=$1
    script_name=./misc/${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    #chmod +x ${script_name}
    python3 ${script_name} --party_id=2 >log/${name}-2.log 2>&1 &
    python3 ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
    time python3 ${script_name} --party_id=0 >log/${name}-0.log 2>&1

    wait
    echo -e "\nrun ${script_name} \033[32;32;1mpass\033[0m"
}

echo -e "\n*** performance of secure ops test running... ****\n"
test_op test_binary_op_perf
echo -e "\n*** performance of secure ops test ${GREEN}pass${NOCOLOR}. ***\n"
