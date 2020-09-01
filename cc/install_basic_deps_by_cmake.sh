#!/bin/bash

echo "$@"

curdir=$(pwd)
workdir=$1
buildtype=$2
thirddir=$3
cmaketasks=8
echo $curdir
echo $workdir
echo $buildtype
echo $thirddir

#
# all necessary dependencies
#
sudo apt install -y libssl-dev libexpat1-dev libgmp-dev libcurl4-openssl-dev

# python dependencies
pip3 install pandas --user


