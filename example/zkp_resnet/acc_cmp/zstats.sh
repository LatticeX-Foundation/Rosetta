#!/bin/bash
#cat 50m-cifar10-resnet50-1.log-mem|awk '{print $3}'
arr=$(ls *mem)
for i in ${arr}; do
  cat ${i} | awk '{print $3}' >${i}.csv
  python3 ./zstats.py ${i}.csv
done
