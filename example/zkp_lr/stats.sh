#!/bin/bash

if [ $# -lt 1 ]; then
  echo "<party id>"
  exit 0
fi

cd log
partyid=$1
dsets=('mnist')
bws=('50m' '200m' '500m' '1000m')
for dset in ${dsets[@]}; do
  for bw in ${bws[@]}; do
    reportlog=report-${bw}-${dset}-logistic.log
    echo "" >${reportlog}

    logfile=${bw}-${dset}-logistic-${partyid}.log
    if [ -f "${logfile}" ]; then
      echo -e "\n\n\n${dset}    logistic P${partyid} all witness" >>${reportlog}
      grep -E '"name":|"cpu":|"bytes-sent":' ${logfile} >>${reportlog}
      grep -E "pystats|secureop|zkstats" ${logfile} >>${reportlog}
    fi

    logfile=${bw}-${dset}-logistic-model-public-${partyid}.log
    if [ -f "${logfile}" ]; then
      echo -e "\n\n\n${dset}    logistic P${partyid} model public" >>${reportlog}
      grep -E '"name":|"cpu":|"bytes-sent":' ${logfile} >>${reportlog}
      grep -E "pystats|secureop|zkstats" ${logfile} >>${reportlog}
    fi

    logfile=${bw}-${dset}-logistic-input-public-${partyid}.log
    if [ -f "${logfile}" ]; then
      echo -e "\n\n\n${dset}    logistic P${partyid} input public" >>${reportlog} >>${reportlog}
      grep -E '"name":|"cpu":|"bytes-sent":' ${logfile} >>${reportlog}
      grep -E "pystats|secureop|zkstats" ${logfile} >>${reportlog}
    fi
  done
done

#
#
#
echo "DONE!"
