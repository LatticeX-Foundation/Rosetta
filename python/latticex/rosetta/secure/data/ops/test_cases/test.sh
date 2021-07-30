#!/bin/bash
#run simple-dataset script
#arg1: test script

script=""
if [ $# -ge 1 ]; then
  script="$1"
fi

if [ ! -e "log" ]; then
  mkdir log
fi

echo "run test $script ..."
python3 $script --party_id=2 > log/p2.log 2>&1 &
python3 $script --party_id=1 > log/p1.log 2>&1 &
python3 $script --party_id=0 > log/p0.log

echo "test $script end."
