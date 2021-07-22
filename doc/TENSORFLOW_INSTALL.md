# TensorFlow Installation Guide

- [TensorFlow Installation Guide](#tensorflow-installation-guide)
  - [Description of documentation](#description-of-documentation)
  - [System requirements](#system-requirements)
  - [System software installation](#system-software-installation)
  - [Binary installation](#binary-installation)
  - [Source code installation](#source-code-installation)
  - [Installation verification](#installation-verification)

## Description of documentation

This document serves as a reference for installing the TensorFlow environment, TensorFlow specified version is `v1.14.0`.

## System requirements

> Tensorflow is consistent with Rosetta's system requirements

- Ubuntu (18.04=)
- Python3 (3.6+)
- Pip3 (19.0+)

## System software installation

- **Check ubuntu**
```bash
lsb_release -r # e.g. Release: 18.04
````
> ***If the output release is not 18.04, the corresponding version of the operating system needs to be installed and then perform the subsequent steps.***

- **Check python3 & pip3**

```bash
python3 --version # e.g. Python 3.6.9
pip3 --version # e.g. pip 20.0.2
````

If the system requirements are not met, perform the installation or upgrade.

```bash
# install python3, pip3, openssl
sudo apt update
sudo apt install python3-dev python3-pip
# upgrade pip3 to latest 
sudo pip3 install --upgrade pip
```

Once the installation is complete, check again for compliance with the system requirements to ensure proper installation.

## Binary installation

The TensorFlow binary installation uses the binary `.whl` package that TensorFlow officially uploads to pypi.

```bash
# Optional, to depress the warning of tensorflow
pip3 install numpy==1.16.4 --user
# install tensorflow
pip3 install tensorflow==1.14.0 --user
````

> Tensorflow binary installation can refer to [official documentation](https://www.tensorflow.org/install/pip)

## Source code installation

> TensorFlow source code is recommended to install more than 100GB of free disk space

1. **Installing python dependency packages**
    ```bash
    pip3 install -U --user pip six wheel setuptools mock 'future>=0.17.1' 'numpy==1.16.4'
    pip3 install -U --user keras_applications --no-deps
    pip3 install -U --user keras_preprocessing --no-deps
    ````

2. **Installation of bazel (v0.25.0)**

    ```bash
    # download bazel binary installer
    wget https://github.com/bazelbuild/bazel/releases/download/0.25.0/bazel-0.25.0-installer-linux-x86_64.sh
    # set to executable
    chmod +x bazel-0.25.0-installer-linux-x86_64.sh
    # install required tool unzip
    sudo apt install unzip
    # install bazel
    ./bazel-0.25.0-installer-linux-x86_64.sh --user
    # update the PATH environment variable
    export PATH="$PATH:$HOME/bin"
    ````

    > bazel installation reference [official documentation](https://docs.bazel.build/versions/master/install-ubuntu.html#install-with-installer-ubuntu)

3. **TensorFlow source code compilation**
    > Installation time will be long (about 6 hours), it is recommended to configure 8G+ memory
    ```bash
    # clone TensorFlow github repository
    git clone https://github.com/tensorflow/tensorflow.git
    # checkout v1.14.0 tag
    cd tensorflow
    git checkout v1.14.0
    # configure and then bazel compile...
    ./configure
    bazel build --config=opt -j 4 //tensorflow/tools/pip_package:build_pip_package
    # build .whl installer
    ./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg
    # install tensorflow
    pip3 install /tmp/tensorflow_pkg/*.whl --user
    ````
    > TensorFlow source code installation can be referenced to [official documentation][tensorflow-source-install] 

## Installation verification

After installation, check TensorFlow availability.

> Note: Please switch to a directory other than the TensorFlow source code for installation verification.

```bash
python3 -c 'import tensorflow as tf;print(tf.__version__)'
````

Output: `v1.14.0` indicates successful installation.

-----

[bazel-install]:https://docs.bazel.build/versions/master/install-ubuntu.html#install-with-installer-ubuntu
[tensorflow-source-install]:https://www.tensorflow.org/install/source