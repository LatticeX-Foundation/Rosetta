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
        export TEST_PROTOCOL=$PROTOCOL
    else
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
    # chmod +x ${script_name}
    python3 ${script_name} --party_id=2 >log/${name}-2.log 2>&1 &
    python3 ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
    python3 ${script_name} --party_id=0 >log/${name}-0.log 2>&1
    sleep 0.5
}

# secure ops for helix and
# test_op secure_tests
# exit 1
# # run utest cases
# echo "run unittest... (cost about 80 seconds)"
# cd utests
# bash utest.sh
# cd ../

# simple standalone op test cases
echo "run standalone test case..."
# test_op tf_const
# test_op axis_test
# test_op abs_test
test_op save_v2
test_op restore_v2
test_op nn_ops_test


# test_op fused_batch_norm

# # test_op print
# test_op matmul
# test_op sigmoid
# test_op add
# test_op sub
# test_op mul
test_op division_test
# test_op relu
# test_op pow_const
# test_op test_xxx_pow


test_op log
test_op log1p


# test_op max
# test_op mean
# test_op sum
# test_op min

# test_op apply_gradient_descent

# test_op greater_equal
# test_op greater
# test_op less_equal
# test_op less
# test_op equal # with equal and not_equal

echo "*** decorator test all Done. ****"
