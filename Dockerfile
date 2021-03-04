FROM ubuntu:18.04

# debconf: unable to initialize frontend: Dialog
ENV DEBIAN_FRONTEND noninteractive

# basic
RUN apt-get update
RUN apt-get install -y --no-install-recommends apt-utils gcc g++ make automake cmake build-essential libssl-dev \
  bsdmainutils libtbb-dev libgmpxx4ldbl && apt-get autoremove
RUN apt-get install -y --no-install-recommends vim wget unzip git tree dos2unix time gawk sudo && apt-get autoremove

# python/pip
RUN apt-get install -y --no-install-recommends python3.7 python3.7-dev python3-distutils python3-pip && apt-get autoremove
RUN cd /usr/bin && ln -sf python3.7 python && ln -sf python3.7 python3 && ln -sf pip3 pip && python -m pip install --upgrade pip
RUN pip3 install setuptools -i https://mirrors.aliyun.com/pypi/simple/
RUN pip3 --no-cache-dir install numpy==1.16.0 pandas sklearn tensorflow==1.14.0 -i https://mirrors.aliyun.com/pypi/simple/

# compile the latest version
RUN cd /home/ && git clone --recurse https://github.com/LatticeX-Foundation/Rosetta.git
RUN cd /home/Rosetta/ && ./rosetta.sh compile --enable-protocol-mpc-securenn && ./rosetta.sh install
RUN cd /home/Rosetta/example/tutorials/code && ./tutorials.sh rtt linear_regression_reveal && tail -n 60 log/linear_regression_reveal-0.log
