FROM tensorflow/tensorflow:1.14.0-py3

# install rosetta from source code

# 1. install python3, pip3, openssl
RUN apt update && apt install -y python3-dev python3-pip libssl-dev cmake git

# 2. install whl deps
RUN yes | pip3 install pandas sklearn

# 3. clone source code, compile
# compile the specified tag version
RUN cd /home/ && git clone https://github.com/LatticeX-Foundation/Rosetta.git && cd Rosetta && git checkout v0.2.1 && bash compile_and_test_all.sh
# compile the latest version
# RUN cd /home/ && git clone https://github.com/LatticeX-Foundation/Rosetta.git && cd Rosetta && bash compile_and_test_all.sh

