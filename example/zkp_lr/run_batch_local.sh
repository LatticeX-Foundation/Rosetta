#!/bin/bash

rm -rf data/*
mkdir -p log data

function kill_prog() {
  sleep 1
  rm -rf data/*
  ps -ef | grep -E "party_id|logistic_regression" | grep -v grep | awk '{print $2}' | xargs kill -9 >/dev/null 2>&1
  sleep 1
}

# train
echo -e "\nrun tf logistic regression   train mnist. ..."
kill_prog
python3 tf-logistic_regression_saver.py >log/mnist_logistic_regression_saver.log 2>&1

# tf predict
echo -e "\nrun tf logistic regression predict mnist. ..."
kill_prog
python3 tf-logistic_regression_restore.py >log/mnist_logistic_regression_restore.log 2>&1

# zk predict
echo -e "\nrun zk logistic regression predict mnist. ..."
kill_prog
python3 rtt-logistic_regression_restore.py --party_id=1 >log/mnist_logistic_regression_restore-1.log 2>&1 &
python3 rtt-logistic_regression_restore.py --party_id=0 >log/mnist_logistic_regression_restore-0.log 2>&1

echo -e "\nrun zk logistic regression predict mnist. model loaded as public ..."
kill_prog
python3 rtt-logistic_regression_restore.py --party_id=1 --model_public --cfgfile=CONFIG-public.json >log/mnist_logistic_regression_restore-model_public-1.log 2>&1 &
python3 rtt-logistic_regression_restore.py --party_id=0 --model_public --cfgfile=CONFIG-public.json >log/mnist_logistic_regression_restore-model_public-0.log 2>&1

echo -e "\nrun zk logistic regression predict mnist. input is public ..."
kill_prog
python3 rtt-logistic_regression_restore.py --party_id=1 --input_public >log/mnist_logistic_regression_restore-input_public-1.log 2>&1 &
python3 rtt-logistic_regression_restore.py --party_id=0 --input_public >log/mnist_logistic_regression_restore-input_public-0.log 2>&1

# error
python3 relative_error.py
