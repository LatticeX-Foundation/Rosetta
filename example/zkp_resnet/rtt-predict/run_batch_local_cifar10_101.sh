#!/bin/bash
rm -rf data/*
mkdir -p log data

function kill_prog() {
  sleep 1
  rm -rf data/*
  ps -ef | grep -E "party_id|resnetx" | grep -v grep | awk '{print $2}' | xargs kill -9 >/dev/null 2>&1
  sleep 1
}

kill_prog
sleep 5
echo -e "\nrun resnet ..."
python3 main.py --phase test --dataset cifar10 --res_n 101 --lr 0.001 \
  --epoch 1 --batch_size 1 --train_size 1 --test_size 1 --party_id=1 >log/cifar10-resnet101-1.log 2>&1 &
python3 main.py --phase test --dataset cifar10 --res_n 101 --lr 0.001 \
  --epoch 1 --batch_size 1 --train_size 1 --test_size 1 --party_id=0 >log/cifar10-resnet101-0.log 2>&1

#
#
#
kill_prog
sleep 5
echo -e "\nrun resnet model loaded as public ..."
python3 main.py --phase test --dataset cifar10 --res_n 101 --lr 0.001 --model_public \
  --epoch 1 --batch_size 1 --train_size 1 --test_size 1 --party_id=1 >log/cifar10-resnet101-model-public-1.log 2>&1 &
python3 main.py --phase test --dataset cifar10 --res_n 101 --lr 0.001 --model_public \
  --epoch 1 --batch_size 1 --train_size 1 --test_size 1 --party_id=0 >log/cifar10-resnet101-model-public-0.log 2>&1

#
#
#
kill_prog
sleep 5
echo -e "\nrun resnet model input as public ..."
python3 main.py --phase test --dataset cifar10 --res_n 101 --lr 0.001 --input_public \
  --epoch 1 --batch_size 1 --train_size 1 --test_size 1 --party_id=1 >log/cifar10-resnet101-input-public-1.log 2>&1 &
python3 main.py --phase test --dataset cifar10 --res_n 101 --lr 0.001 --input_public \
  --epoch 1 --batch_size 1 --train_size 1 --test_size 1 --party_id=0 >log/cifar10-resnet101-input-public-0.log 2>&1

#
#
#
kill_prog
echo "DONE!"
