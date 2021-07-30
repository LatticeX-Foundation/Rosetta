#!/bin/bash

mkdir -p log

# disable dynamic pass
export ROSETTA_DPASS=OFF

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

PROTOCOL="SecureNN"
if [ $# -ge 1 ]; then
    PROTOCOL=$1
    if [[ $PROTOCOL == "SecureNN" ]] || [[ $PROTOCOL == "Helix" ]]; then
        echo "==> do test of $PROTOCOL"
        export ROSETTA_TEST_PROTOCOL=$PROTOCOL
    else
        echo "!!! bad test name, usage: ./prog [Helix|SecureNN]"
        exit 1
    fi
fi

function test_op()
{
    name=$1
    script_name=./simple/${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    # chmod +x ${script_name}
    python3 ${script_name} --party_id=2 >log/${name}-2.log 2>&1 &
    python3 ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
    python3 ${script_name} --party_id=0 >log/${name}-0.log 2>&1
    
    wait
    echo -e "\nrun ${script_name} \033[32;32;1mpass\033[0m"
}

# simple standalone op test cases
echo -e "\n*** simple secure ops test running... ***\n"

# check all op and simple test all secure ops
test_op secure_tests

test_op abs_test
test_op save_v2
test_op saver_test
test_op restore_v2
test_op nn_ops_test
test_op matmul
test_op sigmoid
test_op add
test_op sub
test_op mul
test_op div
test_op reciprocaldiv
test_op relu
test_op pow_const

test_op exp
test_op sqrt
test_op rsqrt

test_op log
test_op log1p

test_op max
test_op mean
test_op sum
test_op min

test_op greater_equal
test_op greater
test_op less_equal
test_op less
test_op equal # with equal and not_equal

test_op logical
test_op relu_prime
test_op relu

test_op apply_gradient_descent

echo -e "\n*** simple secure ops test ${GREEN}pass${NOCOLOR}. ****\n"
