#!/bin/bash

mkdir -p log data

echo -e "\nrun plaintext CIFAR10 test dataset prediction on resnet-50..."
python3 main.py --phase test --res_n 50 --dataset cifar10 --test_size 10000

echo -e "\nrun plaintext CIFAR10 test dataset prediction on resnet-101..."
python3 main.py --phase test --res_n 101 --dataset cifar10 --test_size 10000
