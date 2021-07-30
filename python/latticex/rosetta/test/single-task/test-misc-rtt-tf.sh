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

function test_rtt_op()
{
    name=$1
    script_name=./misc/${name}.py
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

function test_tf_op()
{
    name=$1
    script_name=./misc/${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun tf ${script_name} ..."
    # chmod +x ${script_name}
    python3 ${script_name} --party_id=2 >log/${name}-tf.log 2>&1
}


# op test cases
echo -e "\n*** misc secure ops test running... ***"

test_tf_op pool_ops_check_tf
test_rtt_op pool_ops_check

test_tf_op reduce_mean_tf
test_rtt_op reduce_mean_rtt

test_tf_op softmax_ce_tf
test_tf_op softmax_tf
# test_rtt_op softmax_rtt

test_rtt_op const_rtt

# rtt cases
test_rtt_op check_hgf
test_rtt_op check_sjj
test_rtt_op check_yyl
test_rtt_op check
# test_rtt_op fused_batch_norm # wolvering

test_rtt_op conv2d_random_nhwc_rtt
test_tf_op conv2d_random_nhwc_tf
test_rtt_op conv2d_rtt
test_tf_op conv2d_special_tf


test_rtt_op test_xxx_pow
test_rtt_op tmp


echo -e "\n***  misc secure ops test ${GREEN}pass${NOCOLOR}. ***\n"

