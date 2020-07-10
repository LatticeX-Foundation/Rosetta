#!/bin/bash

mkdir -p log

export LD_LIBRARY_PATH='../../../../build/lib.linux-x86_64-3.6/latticex'

# disable dynamic pass
export ROSETTA_DPASS=OFF

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

function test_op()
{
    name=$1
    script_name=./${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    # chmod +x ${script_name}
    python3 ${script_name} --party_id 2 --cfgfile ./CONFIG2.json >log/${name}-2.log 2>&1 &
    python3 ${script_name} --party_id 1 --cfgfile ./CONFIG1.json >log/${name}-1.log 2>&1 &
    python3 ${script_name} --party_id 0 --cfgfile ./CONFIG0.json >log/${name}-0.log 2>&1
    sleep 0.5
}

test_op secure_op_test
#test_op greater
