#!/bin/bash

# set -x

mkdir -p log

function install_requred() {
    sklearn=$(pip3 show sklearn && pip3 show pandas)
    if [ $? -ne 0 ]; then
        echo "install required python package ..."
        pip3 install sklearn pandas --user
    fi
}

function run_tf() {
    name=$1
    script_name=./tf-${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    python3 ${script_name} >log/${name}-tf.log 2>&1
    sleep 0.5
}

function run_rtt() {
    name=$1
    script_name=./rtt-${name}.py
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

function run_stat() {
    # compare tf and rosetta
    echo -e "\nrun model_plot ..."
    python3 model_plot.py --sname $1
    echo -e "\nrun model_metrixs ..."
    python3 model_metrixs.py --sname $1 --model $2

    sleep 0.5
}

function run_x() {
    #echo "run $@"
    tttt=$1
    name=$2
    metr=$3
    if [ "$tttt" = "tf" ]; then
        run_tf $name
    elif [ "$tttt" = "rtt" ]; then
        run_rtt $name
    elif [ "$tttt" = "stat" ]; then
        run_stat $name $metr
    else
        echo "not supports argument!"
    fi
}

function run_all() {
    # check and install required packages
    install_requred

    echo "run all tutorials examples"
    # #########################################################
    # basic
    run_x rtt quickstart

    # matmul
    run_x tf matmul
    run_x rtt matmul

    # millionaire
    run_x tf millionaire
    run_x rtt millionaire

    # basic & reveal (linear regression)
    run_x tf linear_regression
    run_x rtt linear_regression
    run_x rtt linear_regression_reveal

    # basic & reveal (logistic regression)
    run_x tf logistic_regression
    run_x rtt logistic_regression
    run_x rtt logistic_regression_reveal

    # save/load (linear regression)
    run_x rtt linear_regression_saver
    run_x tf linear_regression_restore

    # save/load (logistic regression)
    run_x rtt logistic_regression_saver
    run_x tf logistic_regression_restore

    # stat (linear regression)
    run_x tf linear_regression_stat
    run_x rtt linear_regression_stat
    run_x stat linear_regression_stat linear

    # stat (logistic regression)
    run_x tf logistic_regression_stat
    run_x rtt logistic_regression_stat
    run_x stat logistic_regression_stat logistic

    # dataset pipeline
    run_x tf ds-lr
    run_x rtt ds-lr
}

function helper() {
    echo "usage:"
    echo "./tutorials.sh <all|tf|rtt|stat> [<name>] [<linear|logistic>]"
}

echo "$@"
if [ $# -eq 0 ]; then
    helper
    exit 1
fi

tt="all"
if [ $# -ge 1 ]; then
    tt=$1
fi

if [ "$tt" = "all" ]; then
    run_all
    exit 0
fi

if [ $# -lt 2 ]; then
    helper
    exit 1
fi
name=$2

metrixs='linear'
if [ "$tt" = "stat" ]; then
    if [ $# -lt 3 ]; then
        helper
        exit 1
    fi
    metrixs=$3
fi

# run one
run_x $tt $name $metrixs

exit 0
