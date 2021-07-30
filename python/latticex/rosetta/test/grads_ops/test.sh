#!/bin/bash

mkdir -p log

# disable dynamic pass
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

function test_grad_op() {
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
    sleep 2

    judge $1
}


test_grad_op mpcsub_grad_case
test_grad_op mpcadd_grad_case
test_grad_op mpcmul_grad_case
test_grad_op mpcdiv_grad_case
test_grad_op mpcpow_grad_case

test_grad_op mpcneg_grad_case
test_grad_op mpcabs_grad_case
test_grad_op mpcsquare_grad_case
test_grad_op mpclog1p_grad_case
test_grad_op mpclog_grad_case

test_grad_op mpcexp_grad_case
test_grad_op mpcsqrt_grad_case
test_grad_op mpcrsqrt_grad_case

test_grad_op mpcmatmul_grad_case

test_grad_op mpcmin_grad_case
test_grad_op mpcmax_grad_case
test_grad_op mpcmean_grad_case
test_grad_op mpcsum_grad_case

test_grad_op mpcsigmoid_grad_case
test_grad_op mpcrelu_grad_case
test_grad_op mpc_nn_grad_case

exit 0
