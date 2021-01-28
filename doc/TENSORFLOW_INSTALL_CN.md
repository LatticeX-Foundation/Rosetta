# TensorFlow安装指南

- [TensorFlow安装指南](#tensorflow安装指南)
  - [文档说明](#文档说明)
  - [系统要求](#系统要求)
  - [系统软件安装](#系统软件安装)
  - [二进制安装](#二进制安装)
  - [源码安装](#源码安装)
  - [安装检验](#安装检验)

## 文档说明

本文档作为安装TensorFlow环境参考，TensorFlow指定版本为`v1.14.0`。

## 系统要求

> Tensorflow与Rosetta的系统要求一致

- Ubuntu (18.04=)
- Python3 (3.6+)
- Pip3 (19.0+)

## 系统软件安装

- **检查ubuntu**

```bash
lsb_release -r      # e.g. Release: 18.04
```

***如果输出发布版本非`18.04`，则需要安装对应版本操作系统，然后执行后续步骤。***

- **检查python3、pip3**

```bash
python3 --version   # e.g. Python 3.6.9
pip3 --version      # e.g. pip 20.0.2
```

如果不符合系统要求，则执行安装或升级：

```bash
# install python3, pip3, openssl
sudo apt update
sudo apt install python3-dev python3-pip
# upgrade pip3 to latest 
sudo pip3 install --upgrade pip
```

安装完成后，再次检查是否符合系统要求，保证安装的正确性。

## 二进制安装

TensorFlow二进制安装使用TensorFlow官方上传到pypi的二进制whl包。

```bash
# Optional, to depress the warning of tensorflow
pip3 install numpy==1.16.4 --user
# install tensorflow
pip3 install tensorflow==1.14.0 --user
```

> Tensorflow二进制安装可以参考[官方文档](https://www.tensorflow.org/install/pip)

## 源码安装

> TensorFlow源码安装前建议磁盘空闲空间100GB以上

1. **安装python依赖包**

    ```bash
    pip3 install -U --user pip six wheel setuptools mock 'future>=0.17.1' 'numpy==1.16.4'
    pip3 install -U --user keras_applications --no-deps
    pip3 install -U --user keras_preprocessing --no-deps
    ```

2. **安装bazel（v0.25.0）**

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
    ```

    > bazel安装参考[官方文档][bazel-install]

3. **TensorFlow源码编译**
    > 安装时间将很长（约6小时），建议配置8G以上的内存

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
    ```

    > TensorFlow源码安装可以参考[官方文档][tensorflow-source-install]

## 安装检验

安装完成后，检验TensorFlow可用性：

> 注意：请切换到TensorFlow源码之外的目录进行安装检验

```bash
python3 -c 'import tensorflow as tf;print(tf.__version__)'
```

输出：`v1.14.0`表示安装成功。

-----

[bazel-install]:https://docs.bazel.build/versions/master/install-ubuntu.html#install-with-installer-ubuntu
[tensorflow-source-install]:https://www.tensorflow.org/install/source
