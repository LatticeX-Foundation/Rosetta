#!/bin/bash
rm -rf data/*
mkdir -p log data

function kill_prog() {
  sleep 1
  rm -rf data/*
  ps -ef | grep -E "party_id|resnetx" | grep -v grep | awk '{print $2}' | xargs kill -9 >/dev/null 2>&1
  sleep 1
}

if [ $# -lt 1 ]; then
  echo "<party id>"
  exit 0
fi

partyid=$1
dsets=('cifar10')
bws=('50m' '200m' '500m' '1000m')
rs=(50 101)
./throttle.sh del
for dset in ${dsets[@]}; do
  for bw in ${bws[@]}; do
    for res_n in ${rs[@]}; do
      kill_prog
      echo "run ${dset} ${bw} ${res_n}"
      ./throttle.sh lan ${bw}

      #
      #
      sleep 5
      kill_prog
      echo "run ${dset} ${bw} ${res_n} private vs private"
      python3 main.py --phase test --dataset ${dset} --res_n ${res_n} --lr 0.001 \
        --epoch 1 --batch_size 1 --train_size 1 --test_size 1 \
        --cfgfile=CONFIG_LAN.json \
        --party_id=${partyid} >log/${bw}-${dset}-resnet${res_n}-${partyid}.log 2>&1

      #
      #
      sleep 5
      kill_prog
      echo "run ${dset} ${bw} ${res_n} private vs public"
      python3 main.py --phase test --dataset ${dset} --res_n ${res_n} --lr 0.001 \
        --epoch 1 --batch_size 1 --train_size 1 --test_size 1 \
        --model_public --cfgfile=CONFIG_LAN.json \
        --party_id=${partyid} >log/${bw}-${dset}-resnet${res_n}-model-public-${partyid}.log 2>&1

      #
      #
      sleep 5
      kill_prog
      echo "run ${dset} ${bw} ${res_n} public vs private"
      python3 main.py --phase test --dataset ${dset} --res_n ${res_n} --lr 0.001 \
        --epoch 1 --batch_size 1 --train_size 1 --test_size 1 \
        --input_public --cfgfile=CONFIG_LAN.json \
        --party_id=${partyid} >log/${bw}-${dset}-resnet${res_n}-input-public-${partyid}.log 2>&1

      #
      #
      sleep 10
    done
  done
done

#
#
#
kill_prog
echo "DONE!"
