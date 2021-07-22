#!/bin/bash

set -x

sudo ps -ef | grep -E "party_id|unittest" | grep python | awk '{print $2}' | sudo xargs kill -9 >/dev/null 2>&1
sudo ps -ef | grep -E "io-tests|psi02|protocol_|secure_|helix|snn_|netio" | grep -v grep | awk '{print $2}' | sudo xargs kill -9 >/dev/null 2>&1
sudo ps -ef | grep -E "tutorials" | grep -v grep | awk '{print $2}' | sudo xargs kill -9 >/dev/null 2>&1
