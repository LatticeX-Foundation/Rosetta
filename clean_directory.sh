#!/bin/bash

#find . -name '*.sh' | xargs chmod 755

# for debug
set -x

# This script for cleaning up this directory and sub directories

curdir=$(pwd)

mkdir -p build/log
find . -name "log" -type d | xargs rm -rf
find . -name "__pycache__" -type d | xargs rm -rf

rm -rf .publish_to_github
rm -rf .eggs
rm -rf build build128 dist tmp

rm -rf cc/tf/dpass/test_cases/ckp
rm -rf python/latticex.egg-info
rm -rf python/latticex/*.so*

rm python/latticex_rosetta.egg-info -rf
rm dist -rf
