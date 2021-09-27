#!/bin/bash

exit 0

resfile='rtt-preds-ResNet50_cifar10.csv'
rm -rf ${resfile}
touch ${resfile}
for ((i = 0; i < 100; i++)); do
  cat rtt-preds-ResNet50_cifar10-${i}.csv >>${resfile}
done

wc ${resfile}

exit 0

resfile='rtt-preds-ResNet101_cifar10.csv'
rm -rf ${resfile}
touch ${resfile}
for ((i = 0; i < 200; i++)); do
  cat rtt-preds-ResNet101_cifar10-${i}.csv >>${resfile}
done

wc ${resfile}

exit 0
