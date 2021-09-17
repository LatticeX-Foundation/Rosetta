#!/bin/bash
rm -rf data/*
mkdir -p log data

function kill_prog() {
  sleep 1
  rm -rf data/*
  ps -ef | grep -E "party_id|resnetx|logistic" | grep -v grep | awk '{print $2}' | xargs kill -9 >/dev/null 2>&1
  sleep 1
}

if [ $# -lt 1 ]; then
  echo "<party id>"
  exit 0
fi

partyid=$1
dsets=('mnist')
bws=('50m' '200m' '500m' '1000m')
./throttle.sh del
for dset in ${dsets[@]}; do
  for bw in ${bws[@]}; do
    kill_prog
    echo "run ${dset} ${bw}"
    ./throttle.sh lan ${bw}

    #
    #
    sleep 5
    kill_prog
    echo "run ${dset} ${bw} private vs private"
    python3 rtt-logistic_regression_restore.py \
      --cfgfile=CONFIG_LAN.json \
      --party_id=${partyid} >log/${bw}-${dset}-logistic-${partyid}.log 2>&1

    #
    #
    sleep 5
    kill_prog
    echo "run ${dset} ${bw} private vs public"
    python3 rtt-logistic_regression_restore.py \
      --model_public --cfgfile=CONFIG_LAN_public.json \
      --party_id=${partyid} >log/${bw}-${dset}-logistic-model-public-${partyid}.log 2>&1

    #
    #
    sleep 5
    kill_prog
    echo "run ${dset} ${bw} public vs private"
    python3 rtt-logistic_regression_restore.py \
      --input_public --cfgfile=CONFIG_LAN.json \
      --party_id=${partyid} >log/${bw}-${dset}-logistic-input-public-${partyid}.log 2>&1

    #
    #
    sleep 10
  done
done

#
#
#
kill_prog
echo "DONE!"
