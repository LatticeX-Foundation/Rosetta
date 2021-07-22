#!/bin/bash

mkdir -p log

# disable dynamic pass
export ROSETTA_DPASS=OFF

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

PROTOCOL="Wolverine"

if [ $# -ge 1 ]; then
    PROTOCOL=$1
    if [[ $PROTOCOL == "SecureNN" ]] || [[ $PROTOCOL == "Helix" ]] || [[ $PROTOCOL == "Wolverine" ]] ; then
        echo "==> [hgf-test] do test of $PROTOCOL ==="
        export TEST_PROTOCOL=$PROTOCOL
    else
        echo "protocol: ${PROTOCOL} "
        echo "!!! bad test name, usage: ./prog [Helix|SecureNN]"
        exit 1
    fi
fi

function test_op()
{
    name=$1
    script_name=./${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi

    echo -e "\nrun ${script_name} ..."
    if [ ${PROTOCOL} == "Wolverine" ]; then
        python3 ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
        python3 ${script_name} --party_id=0 >log/${name}-0.log 2>&1
    else
        python3 ${script_name} --party_id=2 >log/${name}-2.log 2>&1 &
        python3 ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
        python3 ${script_name} --party_id=0 >log/${name}-0.log 2>&1
    fi

    sleep 0.5
}

rm data/* -rf
# secure ops for helix and
# test_op secure_tests
# test_op check_hgf
# test_op addn
# test_op sub
# test_op sigmoid
test_op pool_ops_check

echo "ok."
