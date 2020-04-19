#!/bin/bash

#find . -name '*.sh' | xargs chmod 755

# for debug
set -x

# This script for cleaning up this directory and sub directories

curdir=$(pwd)

mkdir -p build/log
find . -name "log" -type d|xargs rm -rf

rm -rf .eggs
rm -rf build dist tmp

rm -rf python/latticex.egg-info
