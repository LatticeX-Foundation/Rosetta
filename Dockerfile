FROM tensorflow/tensorflow:1.14.0-py3

# install rosetta from source code

# 1. install python3, pip3, openssl
RUN apt update && apt install -y python3.7-dev python3-pip libssl-dev cmake git psmisc vim sudo

# 2. install whl deps
RUN pip3 install --upgrade pip
RUN yes | pip3 install pandas sklearn

# 3. clone source code, compile
# compile the latest version
RUN cd /home/ && git clone --recurse https://github.com/LatticeX-Foundation/Rosetta.git && ls -al 
RUN cd /home/Rosetta/ && ./rosetta.sh compile --enable-protocol-mpc-securenn; ./rosetta.sh install
RUN cd /home/Rosetta/example/tutorials/code && ./tutorials.sh rtt linear_regression_reveal && tail -n 48 log/linear_regression_reveal-0.log
