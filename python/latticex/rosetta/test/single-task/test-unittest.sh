#!/bin/bash
# python unit tests of secure ops

NOCOLOR='\033[0m'
RED='\033[0;31m'
GREEN='\033[0;32m'

status="Pass"
result="Fail"

if [ ! -d log ]; then
	mkdir log
fi


echo -e "\n***  UnitTest of SecureNN   ***\n"
export ROSETTA_TEST_PROTOCOL=SecureNN

python3 ./utests/unittest_all.py > log/securenn.utest.log 2>&1
result=`tail -n 10 log/securenn.utest.log | grep "OK"`
if [ ${result} == "OK" ]; then
   status="${GREEN}pass${NOCOLOR}"
else
   status="${RED}fail${NOCOLOR}"
fi
echo -e "\n***   UnitTest SecureNN ${status} ***\n"

echo -e "\n***  UnitTest of Helix  ***\n"
export ROSETTA_TEST_PROTOCOL=Helix

python3 ./utests/unittest_all.py > log/helix.utest.log 2>&1
result=`tail -n 10 log/helix.utest.log | grep "OK"`
if [ ${result} == "OK" ]; then
   status="${GREEN}pass${NOCOLOR}"
else
   status="${RED}fail${NOCOLOR}"
fi
echo -e "\n***  UnitTest Helix ${status}  ***\n"


