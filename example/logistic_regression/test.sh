#!/bin/bash

#set -x
mkdir -p log

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

# disable dynamic pass

function test_mpc_llr() {
    if [ $# -lt 3 ]; then
        echo "invalid input, should with 3 argments: name, epochs, dims !"
        return 1
    fi

    name=$1
    epochs=$2
    dims=$3
    lr=$4

    script_name=./rtt/${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun mpc case: ${script_name} ..."

    python3 ${script_name} --party_id=2 --epochs=${epochs} --dims=${dims} --learn_rate=${lr} >log/D${dims}-E${epochs}-LR${lr}-2.log 2>&1 &
    python3 ${script_name} --party_id=1 --epochs=${epochs} --dims=${dims} --learn_rate=${lr} >log/D${dims}-E${epochs}-LR${lr}-1.log 2>&1 &
    python3 ${script_name} --party_id=0 --epochs=${epochs} --dims=${dims} --learn_rate=${lr} >log/D${dims}-E${epochs}-LR${lr}-0.log 2>&1
    sleep 0.5
}

function test_plain_llr() {
    name=$1
    epochs=$2
    dims=$3
    lr=$4

    script_name="./tf/${name}".py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun plain case: ${script_name} ..."

    python3 ${script_name} --epochs=${epochs} --dims=${dims} --learn_rate=${lr} > log/D${dims}-E${epochs}-LR${lr}-plain.log 2>&1
}

function get_trained_params() {
    name=$1
    epochs=$2
    dims=$3
    lr=$4

    logmpc="./log/D${dims}-E${epochs}-LR${lr}-0.log"
    #logmpc1="./log/D${dims}-E${epochs}-LR${lr}-1.log"
    logplain="./log/D${dims}-E${epochs}-LR${lr}-plain.log"

    #grep "trained-param" ${logplain}
    plain_W=`grep "trained-param" ${logplain} |grep W|awk -F ":" '{print $2}'`
    plain_b=`grep "trained-param" ${logplain} |grep b|awk -F ":" '{print $2}'`
    plain_acc=`grep "trained-acc" ${logplain} |grep acc|awk -F ":" '{print $2}'`
    echo "---------------  ${name}-D${dims}-E${epochs}-LR${lr} plain params: ----------------"
    echo -e "${GREEN}W:${NOCOLOR} "$'\n'"${plain_W}"
    echo -e "${GREEN}b:${NOCOLOR} "$'\n'"${plain_b}"
    echo -e "${GREEN}acc:${NOCOLOR} "$'\n'"${plain_acc}"

    mpc_W=`grep "trained-param" ${logmpc} |grep W|awk -F ":" '{print $2}'`
    mpc_b=`grep "trained-param" ${logmpc} |grep b|awk -F ":" '{print $2}'`
    mpc_acc=`grep "trained-acc" ${logmpc} |grep acc|awk -F ":" '{print $2}'`
    echo "---------------  ${name}-D${dims}-E${epochs}-LR${lr} mpc params: ----------------"
    echo -e "${GREEN}W:${NOCOLOR} "$'\n'"${mpc_W}"
    echo -e "${GREEN}b: ${NOCOLOR}"$'\n'"${mpc_b}"
    echo -e "${GREEN}acc:${NOCOLOR} "$'\n'"${mpc_acc}"

    delta_W=`python3 -c "import numpy as np;print(np.double($plain_W)-np.double($mpc_W))"`
    delta_b=`python3 -c "import numpy as np;print(np.double($plain_b)-np.double($mpc_b))"`
    delta_acc=`python3 -c "import numpy as np;print(np.double($plain_acc)-np.double($mpc_acc), '%')"`
    echo -e "${name}-D${dims}-E${epochs}-LR${lr} 'plain-mpc' ${GREEN}delta_W:${NOCOLOR}: "$'\n'"${RED} ${delta_W} ${NOCOLOR}"
    echo -e "${name}-D${dims}-E${epochs}-LR${lr} 'plain-mpc' ${GREEN}delta_b:${NOCOLOR}: "$'\n'"${RED} ${delta_b} ${NOCOLOR}"
    echo -e "${name}-D${dims}-E${epochs}-LR${lr} 'plain-mpc' ${GREEN}delta_acc:${NOCOLOR}: "$'\n'"${RED} ${delta_acc} ${NOCOLOR}"
}


#set -x

test_learn_rates="0.01" # "0.1 0.03 0.01 0.003 0.001 0.0003 0.00015"
test_epochs="50" # "1 6 10 20 30 50 80 130 210"
test_dims="5 17" # "1 2 3 5 17"
for lr in ${test_learn_rates}; do
    for dim in ${test_dims}; do
        for epoch in ${test_epochs}; do
            test_mpc_llr "llr_mse" ${epoch} ${dim} ${lr}
            test_plain_llr "llr_mse" ${epoch} ${dim} ${lr}
            get_trained_params "llr_mse" ${epoch} ${dim} ${lr} | tee log/mse-d${dims}-e${epoch}-lr${lr}.stat
        done
    done
done

echo "ok."


