# Rosetta部署指南

- [Rosetta部署指南](#rosetta%e9%83%a8%e7%bd%b2%e6%8c%87%e5%8d%97)
  - [文档说明](#%e6%96%87%e6%a1%a3%e8%af%b4%e6%98%8e)
  - [系统要求](#%e7%b3%bb%e7%bb%9f%e8%a6%81%e6%b1%82)
    - [系统组件](#%e7%b3%bb%e7%bb%9f%e7%bb%84%e4%bb%b6)
    - [TensorFlow](#tensorflow)
  - [安装](#%e5%ae%89%e8%a3%85)
    - [Rosett源码安装](#rosetta)
  - [部署测试](#%e9%83%a8%e7%bd%b2%e6%b5%8b%e8%af%95)
    - [范例](#%e8%8c%83%e4%be%8b)
    - [准备](#%e5%87%86%e5%a4%87)
    - [配置](#%e9%85%8d%e7%bd%ae)
    - [运行测试](#%e8%bf%90%e8%a1%8c%e6%b5%8b%e8%af%95)
      - [单机测试](#%e5%8d%95%e6%9c%ba%e6%b5%8b%e8%af%95)
      - [多机测试](#%e5%a4%9a%e6%9c%ba%e6%b5%8b%e8%af%95)

----

## 文档说明

本文将说明如何安装Rosetta开发环境，部署和测试隐私机器学习应用。

## 系统要求

> 目前，Rosetta 可以运行在 Ubuntu 18.04 操作系统下，并且基于 TensorFlow 1.14 CPU 版本开发。后续测试充分后，将支持更多版本系统。

- Ubuntu (18.04=)
- Python3 (3.6+)
- Pip3 (19.0+)
- Openssl (1.1.1+)
- Tensorflow (1.14.0=, cpu-only)
- CMake（3.10+）
- Rosetta (latest)

如果您本地系统的基础环境已经符合如上要求，可跳过下述`系统组件`检查步骤，直接安装Rosetta包。

### 系统组件

- **Ubuntu**:
  检查版本:

  ```bash
  lsb_release -r      # e.g. Release: 18.04
  ```

  > ***注意：如果输出发布版本好比`18.04`小，则需要升级操作系统，然后执行后续步骤。***

- **Python3 & Pip3 & Openssl & CMake**
  检查版本:

  ```bash
  python3 --version   # e.g. Python 3.6.9
  pip3 --version      # e.g. pip 20.0.2
  apt show libssl-dev # e.g. Version: 1.1.1-1ubuntu2.1~18.04.5
  cmake --version     # e.g. cmake version 3.15.2
  ```

  如果不符合系统要求，则执行以下步骤:

  ```bash
  # install python3, pip3, openssl
  sudo apt update
  sudo apt install python3-dev python3-pip libssl-dev cmake
  # upgrade pip3 to latest 
  sudo pip3 install --upgrade pip
  ```

  请确保安装环境符合要求。

### TensorFlow

  使用如下命令行安装原生 TensorFlow 库。

  ```bash
  # install tensorflow
  pip3 install tensorflow==1.14.0
  ```

### Rosetta

安装Rosetta当前仅支持源码方式安装（如下Rosetta源码安装），二进制方式即将推出。

## Rosetta源码安装

编译源码和安装：

```bash
# clone rosetta git repository
git clone https://github.com/LatticeX-Foundation/Rosetta.git --recursive
# go to Rosetta directory and use auto completion
cd Rosetta && source rtt_completion
# compile, install and run test cases
./rosetta.sh compile --enable-all --enable-tests;./rosetta.sh install
```

## 部署测试

安装好`Rosetta`开发环境，接下对[百万富翁问题][millionaire-problem]构建范例测试`Rosetta`可用性。`Rosetta`使用的是三方`MPC`模型，需要部署三个计算节点，可以单机器或多机器部署。

> Rosetta基于TensorFlow实现，当前只支持TensorFlow的本地部署。

### 范例

直接使用`Rosetta`源码仓库的[百万富翁问题范例][millionaire-example]。

> 注意: [Rosetta开发教程][tutorials]有多个隐私机器学习开发实例可以参考，详情参考[Tutorials](./TUTORIALS.md)。

### 准备

为三个计算节点`P0`、`P1`、`P2`分别创建工作目录，比如: `millionaire0`、`millionaire1`、`millionaire2`

```bash
mkdir millionaire0 millionaire1 millionaire2
```

- 下载百万富翁问题范例

下载范例[Python程序](../example/millionaire/millionaire.py) 到`millionaire0`、`millionaire1`、`millionaire2`工作目录.

```bash
wget https://github.com/LatticeX-Foundation/Rosetta/tree/master/example/millionaire/millionaire.py
```

- 生成证书和key
`P0`、`P1`、`P2`分别生成ssl服务端证书和key，执行命令:

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

> 注意: 实环境部署，建议使用第三方可信证书。

### 配置

编写配置文件，配置文件模版如下:

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
    "RESTORE_MODE": 0
  }
}
```

字段说明:

- `PARTY_ID`: 计算节点参与的角色ID，可取0，1，2，分别对应`P0`、`P1`、`P2`
- `MPC`: 指定为安全多方计算协议配置
- `FLOAT_PRECISION`: 安全多方浮点计算的精度位数
- `P0`、`P1`、`P2`: 别为三方`MPC`联合训练player `P0`，`P1`，`P2`
- `NAME`: `MPC` player名字标识
- `HOST`: 主机地址
- `PORT`: 通信端口
- `SAVER_MODE`: 模型保存配置值，可以通过此值的配置设定保存的模型中的参数值是否为明文值，或者在具体哪一参与方中保存为明文，具体请参考[算子API文档](API_DOC_CN.md)
- `RESTORE_MODE`: 模型加载方式，按照位设定模型位明文或密文，0：表示的密文，1：标示明文，如取值0：所有参与方都为密文方式，1：除P0外所有参与方都是密文方式，2：除P1外所有参与方都是密文方式

### 运行测试

#### 单机测试

`P0`、`P1`、`P2`分别在`millionaire0`、`millionaire1`、`millionaire2`目录下进行测试，使用模版配置并保存为CONFIG.json。

运行`百万富翁问题`范例:

> 注意：运行过程将提示控制台输入值

- **`P2`节点**

```bash
mkdir log
# MPC player 2
python3 millionaire.py --party_id=2
```

- **`P1`节点**

```bash
mkdir log
# MPC player 1
python3 millionaire.py --party_id=1
```

- **`P0`节点**

```bash
mkdir log
# MPC player 0
python3 millionaire.py --party_id=0
```

执行完成后，如果可以查看到类似输出:

```bash
-------------------------
1.0
-------------------------
```

则表示执行正确，单机部署测试成功，否则部署失败。

#### 多机测试

多机测试类似于单机测试，不同点在于配置文件需要设置不同的`HOST`字段为对应IP地址。

----

[tensorFlow-install]:TENSORFLOW_INSTALL_CN.md
[millionaire-problem]:https://en.wikipedia.org/wiki/Yao%27s_Millionaires%27_Problem
[millionaire-example]:../example/millionaire/millionaire.py
[tutorials]:TUTORIALS.md
