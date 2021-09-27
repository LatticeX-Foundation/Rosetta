#!/bin/bash
rm -rf data/*
mkdir -p logs log data

function kill_prog() {
  sleep 1
  rm -rf data/*
  ps -ef | grep -E "party_id|resnetx" | grep -v grep | awk '{print $2}' | xargs kill -9 >/dev/null 2>&1
  sleep 1
}

sleep 3
kill_prog

mkdir -p log/res
test_size=1
echo -e "\nrun resnet ..."
python3 main.py --phase test --dataset cifar10 --res_n 50 --lr 0.001 \
  --epoch 1 --batch_size 100 --train_size 1 --test_size ${test_size} --party_id=1 >log/cifar10-resnet50-1.log 2>&1 &
python3 main.py --phase test --dataset cifar10 --res_n 50 --lr 0.001 \
  --epoch 1 --batch_size 100 --train_size 1 --test_size ${test_size} --party_id=0 >log/cifar10-resnet50-0.log 2>&1

#
#
#
kill_prog
echo "DONE!"
