#!/bin/bash

mkdir -p log

export ROSETTA_DPASS=OFF
NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

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

function test_spass() {
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

    wait
    # sleep 0.5

    judge $1
}

function test_zk_ruu() {
    name=$1
    script_name=./${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    # chmod +x ${script_name}
    python3 ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
    python3 ${script_name} --party_id=0 >log/${name}-0.log 2>&1

    wait
    # sleep 0.5
    judge $1
}

echo -e "\n*** spass test ...***"
test_spass test_node_dup
test_spass test_getitem
test_spass test_conv2d
test_spass test_iter2
test_spass test_iter
test_spass test_di
test_spass test_graph_opt
test_spass test_graph_opt2
test_spass test_graph_opt3
## test_spass test_mpc_thread
test_spass test_assign
test_spass test_mpc_feed
test_spass test_mpc_feed2
test_spass test_mpc_reveal
test_spass test_mpc_opt
test_spass test_mpc_truncated_normal
test_spass test_mpc_random_uniform
test_spass test_mpc_normal_random
test_spass test_mpc_neg_op
test_spass test_mpc_abs_op
test_spass test_mpc_square_op
test_spass test_mpc_add_op
test_spass test_mpc_add2_op
test_spass test_mpc_sub2_op
test_spass test_mpc_sub_op
test_spass test_mpc_placeholder
test_spass test_mpc_div_op
test_spass test_mpc_log1p_op
test_spass test_mpc_log_op
test_spass test_mpc_exp_op
test_spass test_mpc_sqrt_op
test_spass test_mpc_rsqrt_op
test_spass test_mpc_matmul_op
test_spass test_mpc_max2_op
test_spass test_mpc_max_op
test_spass test_mpc_mean2_op
test_spass test_mpc_mean_op
test_spass test_mpc_mul_op
test_spass test_mpc_pow2_op
test_spass test_mpc_pow_op
test_spass test_mpc_sigmoid_op
test_spass test_mpc_relu_op
test_spass test_mpc_equal_op
test_spass test_mpc_notequal_op
test_spass test_mpc_greaterequal_op
test_spass test_mpc_greater_op
test_spass test_mpc_lessequal_op
test_spass test_mpc_less_op
test_spass test_mpc_logical_and_op
test_spass test_mpc_logical_or_op
test_spass test_mpc_logical_xor_op
test_spass test_mpc_logical_not_op
test_spass test_mpc_predict
test_spass LR
test_spass LR2
test_spass Logistic
test_spass test_resnetX
test_spass test_resnet50

# test_zk_ruu test_softmax

echo -e "\n*** spass test ${GREEN}pass${NOCOLOR}.***\n"
