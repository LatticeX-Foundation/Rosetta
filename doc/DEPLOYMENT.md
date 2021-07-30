# Rosetta Deployment Guide

- [Rosetta Deployment Guide](#rosetta-deployment-guide)
  - [System requirements](#system-requirements)
  - [Installation](#installation)
    - [System components](#system-components)
    - [TensorFlow](#tensorflow)
    - [Rosetta](#rosetta)
      - [Source code installation](#source-code-installation)
  - [Deployment testing](#deployment-testing)
    - [Example](#example)
  - [Preparation](#preparation)
    - [Configuration](#configuration)
  - [Run the test](#run-the-test)
    - [Stand-alone testing](#stand-alone-testing)
    - [Multi-machine testing](#multi-machine-testing)

----

In this document, we will describe how to install the Rosetta library with source code from scratch, and how to set up the environment to deploy and test the privacy-preserving machine learning application.

## System requirements

> Currently, Only Ubuntu 18.04 is supported. We will strive to support various systems in the near future after thorough testing.

- Ubuntu (18.04=)
- Python3 (3.6+)
- Pip3 (19.0+)
- Openssl (1.1.1+)
- TensorFlow (1.14.0=, cpu-only)
- CMake (3.10+)
- Rosetta (latest)

## Installation

### System components

- **Ubuntu**:   
  Check version: 

  ```sh
  lsb_release -r         # e.g. Release: 18.04
  ```
  
  > ***Note: If your OS version is less than 18.04, then you should upgrade your operating system and then continue the following  steps***

- **Python3 & Pip3 & Openssl & CMake**
  Check the version:   

  ```sh
  python3 --version     # e.g. Python 3.6.9
  pip3 --version        # e.g. pip 20.0.2
  apt show libssl-dev   # e.g. Version: 1.1.1-1ubuntu2.1~18.04.5
  cmake --version       # e.g. cmake version 3.15.2
  ```

  If the above software versions are not met, you may install or upgrade them as follows: 
  ```sh
  # install python3, pip3, openssl
  sudo apt update
  sudo apt install python3-dev python3-pip libssl-dev cmake
  # upgrade pip3 to latest 
  sudo pip3 install --upgrade pip
  ```

  After the above installation, please check their version again so that we have a suitable base environment.

### TensorFlow

We leave the details of TensorFlow installation in a separate document:[TensorFlow Installation][tensorflow-install], please refer to it if needed.

### Rosetta

Currently, `Rosetta` is only supported to be installed from source code. We are working on its installation with binary packages, and  will release it as soon as possible.

#### Source code installation

Since we have wrapped all the steps in a script, so just get the source code and install it as follows:

```bash
# clone rosetta git repository
git clone https://github.com/LatticeX-Foundation/Rosetta.git --recursive
# compile, install and run test cases
cd Rosetta && bash compile_and_test_all.sh
```

## Deployment testing

After Installing Rosetta, we can test whether it works or not. We can do this by building a demo of the [millionaire problem][millionaire-problem]. `Rosetta` uses a 3-party `MPC` model that requires the deployment of three computing nodes, which can be deployed on real multiple machines or be simulated in s single machine with multi-processes.

> `Rosetta` is based on TensorFlow. Currently, it only supports non-distributed graph execution.

### Example

Here we use the famous [demo of millionaire's problem][millionaire-example] as our example.


> We can simulate the stand-alone deployment scenario of this example by running `run.sh` script in that directory.

## Preparation

Create separate directories for three computing nodes `P0`, `P1`, `P2`, e.g. `millionaire0`, `millionaire1`, `millionaire2`. 
```bash
mkdir millionaire0 millionaire1 millionaire2
```
- Download the demo

Download the [python script](../example/millionaire/millionaire.py) to `millionaire0`, `millionaire1`, `millionaire2` directory for `P0`, `P1`, `P2` respectively.

```bash
wget https://github.com/LatticeX-Foundation/Rosetta/tree/master/example/millionaire/millionaire.py
```

- Generate server key and certificate

`P0`, `P1`, `P2` nodes need generate their separate ssl server certificate and private key respectively, execute the command below: 

```bash
mkdir certs
# generate private key
openssl genrsa -out certs/server-prikey 4096
# if ~/.rnd not exists, generate it with `openssl rand`
if [ ! -f "${HOME}/.rnd" ]; then openssl rand -writerand ${HOME}/.rnd; fi
# generate sign request
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Rosetta SSL IO server/OU=Rosetta server unit/CN=server' -key certs/server-prikey -out certs/cert.req
# sign certificate with cert.req
openssl x509 -req -days 365 -in certs/cert.req -signkey certs/server-prikey -out certs/server-nopass.cert
```

> **Note: We you deploy your system with Rosetta in production environment, be certain to use real trusted third-party certificates.**

### Configuration

Write a configuration file `CONFIG.json` with the following template: 
```json
{
  "PARTY_ID": 0,
  "MPC": {
    "FLOAT_PRECISION": 16,
    "P0": {
      "NAME": "PartyA(P0)",
      "HOST": "127.0.0.1",
      "PORT": 11121
    },
    "P1": {
      "NAME": "PartyB(P1)",
      "HOST": "127.0.0.1",
      "PORT": 12144
    },
    "P2": {
      "NAME": "PartyC(P2)",
      "HOST": "127.0.0.1",
      "PORT": 13169
    },
    "SAVER_MODE": 7,
    "SERVER_CERT": "certs/server-nopass.cert",
    "SERVER_PRIKEY": "certs/server-prikey",
    "SERVER_PRIKEY_PASSWORD": "123456"
  }
}
```
Field Description: 
- `PARTY_ID`: role of Secure Multipart Computation, the valid values are 0,1,2, corresponding to `P0`, `P1`, `P2` respectively
- `MPC`: specify the protocol of Secure Multipart Computation
- `FLOAT_PRECISION`: the float-point precision of Secure Multipart Computation
- `P0`, `P1`, `P2`: `Three-Parties-MPC` players `P0`, `P1`, `P2`
- `NAME`: `MPC` player name tag
- `HOST`: host address
- `PORT`: communication port
- `SERVER_CERT`: server-side signature certificate
- `SERVER_PRIKEY`: server private key
- `SERVER_PRIKEY_PASSWORD`: server private key password (empty string if not set)
- `SAVER_MODE`: this indicates how the output checkpoint files are saved. Please refer to `MpcSaveV2` in our [API document](./API_DOC.md) for details.


## Run the test

### Stand-alone testing

Perform stand-alone testing in the Millionaire directory, Firstly, configure the configuration file using the template and save it as CONFIG.json.

Run the `Millionaire Problem` example:

> Note: The console will be prompted for your private inputs at the beginning.

- **`P2`node**

```bash
mkdir log
# MPC player 2
python3 millionaire.py --party_id=2
```

- **`P1`node**

```bash
mkdir log
# MPC player 1
python3 millionaire.py --party_id=1
```

- **`P0` node**

```bash
mkdir log
# MPC player 0
python3 millionaire.py --party_id=0
```

After execution, output should be like this: 
```bash
-------------------------------------------------
1.0
-------------------------------------------------
```

It means that your example has run smoothly and the standalone deployment test has passed, otherwise the test has failed, and please check the above deployment steps.


### Multi-machine testing

Multi-machine testing is similar to stand-alone testing, with the difference that the configuration file needs to be set to a different `HOST` field corresponding to the IP address.


-----

[tensorFlow-install]:TENSORFLOW_INSTALL.md
[millionaire-problem]:https://en.wikipedia.org/wiki/Yao%27s_Millionaires%27_Problem
[millionaire-example]:../example/millionaire/millionaire.py
[tutorials]:TUTORIALS.md
