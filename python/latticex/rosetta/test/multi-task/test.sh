#!/bin/bash

mkdir -p log


function judge() {
    name=$1
    script_name=./${name}.py
    log_name=./log/${name}-0.log
    message='Fail$'
    if cat ${log_name} | grep "$message" >/dev/null; then
        echo -e "\nrun ${script_name} \033[31;31;1mfail\033[0m"
        return
    else
        echo -e "\nrun ${script_name} \033[32;32;1mpass\033[0m"
        return

    fi
}


function test_mt_task() {
    name=$1
    script_name=./${name}.py
    if [ ! -f ${script_name} ]; then
        echo "${script_name} not exist!"
        return
    fi
    echo -e "\nrun ${script_name} ..."
    python3 -u ${script_name} --party_id=2 >log/${name}-2.log 2>&1 &
    python3 -u ${script_name} --party_id=1 >log/${name}-1.log 2>&1 &
    python3 -u ${script_name} --party_id=0 >log/${name}-0.log 2>&1
    
    #sleep 2
    wait

    judge $1
}


# function unit_test() {
#     exclude_files=("./sqrt.py", "./rsqrt.py", "./exp.py", "./argmax.py", "./softmax.py")

#     filelist=`find  ./ -name "*.py"`
#     for file in $filelist
#     do  
#         if ! [[ "${exclude_files[@]}" =~ "$file" ]]
#         then
#             test_mt_task ${file%*${file:(-3)}}
#         fi
#     done
# }
# unit_test


test_mt_task add
test_mt_task sub
test_mt_task mul
test_mt_task div
test_mt_task matmul
test_mt_task pow
test_mt_task neg
test_mt_task square
test_mt_task log
test_mt_task log1p
test_mt_task abs
test_mt_task equal
test_mt_task nequal
test_mt_task less
test_mt_task greater
test_mt_task less_equal
test_mt_task greater_equal
test_mt_task logic_and
test_mt_task logic_or
test_mt_task logic_xor
test_mt_task logic_not
test_mt_task reduce_min
test_mt_task reduce_max
test_mt_task reduce_mean
test_mt_task reduce_sum
test_mt_task sigmoid
test_mt_task relu
test_mt_task sqrt
test_mt_task rsqrt
test_mt_task exp
# test_mt_task argmax
# test_mt_task softmax
