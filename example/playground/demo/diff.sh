#!/bin/bash

mkdir -p log

function test_lr_tf() {
    name=$1
    epochs=$2
    dims=$3
    script_name=./tf/${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    python ${script_name} --epochs=${epochs} --dims=${dims} >log/${name}-tf.log 2>&1
    sleep 0.5
}

function test_lr_rtt() {
    name=$1
    epochs=$2
    dims=$3
    script_name=./rtt/${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    # chmod +x ${script_name}
    python ${script_name} --party_id=2 --epochs=${epochs} --dims=${dims} >log/${name}-2.log 2>&1 &
    python ${script_name} --party_id=1 --epochs=${epochs} --dims=${dims} >log/${name}-1.log 2>&1 &
    python ${script_name} --party_id=0 --epochs=${epochs} --dims=${dims} >log/${name}-0.log 2>&1
    sleep 0.5
}

# <script-name> <epoch>
function test_LR() {
    # tensorflow version
    test_lr_tf $@

    # rosetta version
    test_lr_rtt $@

    # compare tf and rosetta
    echo -e "\nrun lr_diff ..."
    python LR_SCE_diff.py --epochs $2 --dim $3

    sleep 0.5
}

# simplest
#test_lr_tf LR_with_SCE 100 2
#test_lr_rtt LR_with_SCE 100 2

# simplest with placeholder
#test_lr_tf lr_simplest_placeho 10 5
#test_lr_rtt lr_simplest_placeholder 10 5

# simplest with statistic
test_LR LR_with_SCE 100 2

exit 0
