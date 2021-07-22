#!/bin/bash

if [ $# -lt 1 ]; then
  echo "<party id>"
  exit 0
fi

cd log
partyid=$1
dsets=('cifar10')
bws=('50m' '200m' '500m' '1000m')
rs=(50 101)
for dset in ${dsets[@]}; do
  for bw in ${bws[@]}; do
    for res_n in ${rs[@]}; do
      reportlog=report-${bw}-${dset}-resnet${res_n}.log
      echo "" >${reportlog}

      logfile=${bw}-${dset}-resnet${res_n}-${partyid}.log
      if [ -f "${logfile}" ]; then
        echo -e "\n\n\n${dset}    resnet${res_n} P${partyid} all witness" >>${reportlog}
        grep -E '"name":|"cpu":|"bytes-sent":' ${logfile} >>${reportlog}
        grep -E "pystats|secureop|zkstats" ${logfile} >>${reportlog}
      fi

      logfile=${bw}-${dset}-resnet${res_n}-model-public-${partyid}.log
      if [ -f "${logfile}" ]; then
        echo -e "\n\n\n${dset}    resnet${res_n} P${partyid} model public" >>${reportlog}
        grep -E '"name":|"cpu":|"bytes-sent":' ${logfile} >>${reportlog}
        grep -E "pystats|secureop|zkstats" ${logfile} >>${reportlog}
      fi

      logfile=${bw}-${dset}-resnet${res_n}-input-public-${partyid}.log
      if [ -f "${logfile}" ]; then
        echo -e "\n\n\n${dset}    resnet${res_n} P${partyid} input public" >>${reportlog} >>${reportlog}
        grep -E '"name":|"cpu":|"bytes-sent":' ${logfile} >>${reportlog}
        grep -E "pystats|secureop|zkstats" ${logfile} >>${reportlog}
      fi
    done
  done
done

#
#
#
echo "DONE!"
