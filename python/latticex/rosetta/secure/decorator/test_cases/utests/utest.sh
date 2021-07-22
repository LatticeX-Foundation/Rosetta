#!/bin/bash
# python unit tests of secure ops

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

status="OK"
result="fail"

if [ ! -d log ]; then
	mkdir log
fi

echo "***************   UnitTest SecureNN "
export TEST_PROTOCOL=SecureNN
python3 unittest_all.py > log/SecureNN.unit.log 2>&1
result=`tail -n 10 log/SecureNN.unit.log | grep "OK"`
if [ ${result} == "OK" ]; then
   status="${GREEN}pass${NOCOLOR}"
else
   status="${RED}fail${NOCOLOR}"
fi
echo -e "***************   UnitTest SecureNN ${status} "

echo "***************   UnitTest Helix "
export TEST_PROTOCOL=Helix
python3 unittest_all.py > log/helix.unit.log 2>&1
result=`tail -n 10 log/helix.unit.log | grep "OK"`
if [ ${result} == "OK" ]; then
   status="${GREEN}pass${NOCOLOR}"
else
   status="${RED}fail${NOCOLOR}"
fi
echo -e "***************   UnitTest Helix ${status} "


