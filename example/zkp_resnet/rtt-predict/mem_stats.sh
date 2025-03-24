#!/bin/bash

party_id=1
dset=cifar10
interval=0.5

mkdir -p logx
echo "time pid vmrss(kB) cpu(%)" >logx/meminfo.log

while true; do
    pid=$(ps -ef | grep -v grep | grep dataset | grep python | grep "party_id=${party_id}" | grep ${dset} | awk '{print $2}')
    if [ ! ${pid} ]; then
        sleep 1
        continue
    fi
    perffile=logx/meminfo-${pid}.log
    touch ${perffile}
    localtime=$(date +%y-%m-%d-%H:%M:%S)
    mvrss=$(cat /proc/${pid}/status | grep -e VmRSS | awk '{print $2;}')
    cpupercent=$(top -n 1 -p ${pid} | grep ${pid} | awk '{ssd=NF-4} {print $ssd}')
    echo "${localtime} ${pid} ${mvrss} ${cpupercent}" >>${perffile}
    sleep ${interval}
done
