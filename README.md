![LOGO](https://mmbiz.qpic.cn/mmbiz_png/dV0Pt26LydDKo3HFIeH8afhT8XCmZibWhmj4vuVyuyGQrb0U4vIicibd5xjQKPOib7ibhFRWia9mdbz8uyricY9ZbDgXg/640)

[![github license](https://img.shields.io/badge/license-LGPLv3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

[![Build-and-Test](https://github.com/LatticeX-Foundation/Rosetta/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/LatticeX-Foundation/Rosetta/actions/workflows/build-and-test.yml) [![Performance Test](https://github.com/LatticeX-Foundation/Rosetta/actions/workflows/performance-test.yml/badge.svg)](https://github.com/LatticeX-Foundation/Rosetta/actions/workflows/performance-test.yml)

------

[中文版](./README_CN.md)

## Overview

Rosetta is a privacy-preserving framework based on [TensorFlow](https://www.tensorflow.org). It integrates with mainstream privacy-preserving computation technologies, including cryptography, federated learning and trusted execution environment. Rosetta aims to provide privacy-preserving solutions for artificial intelligence without requiring expertise in cryptography, federated learning and trusted execution environment. Rosetta reuses the APIs of TensorFlow and allows to transfer traditional TensorFlow codes into a privacy-preserving manner with minimal changes. E.g., just add the following line.

```python
import latticex.rosetta
```

The current version integrates the secure multi-party computation protocols for 3 parties. The underlying protocol is [SecureNN](https://eprint.iacr.org/2018/442.pdf). It is secure in the semi-honest model with honest majority. And we are integrating more MPC protocols continuously.

## Installation

For now, Rosetta runs on Ubuntu 18.04, and is based on TensorFlow 1.14 with CPUs (Windows OS is not currently supported yet). You can install Rosetta as follows.

First, please check that your local system meets our [base environment requirement](doc/DEPLOYMENT.md#rosetta-deployment-guide).

Then install the native TensorFlow with the following codes. Note that you could also install it from source code, check [here](doc/TENSORFLOW_INSTALL.md) for details.

```bash
# install tensorflow
pip3 install tensorflow==1.14.0
```

And then build and install Rosetta with our all-in-one script as follows.

```bash
# clone rosetta git repository
git clone --recurse https://github.com/LatticeX-Foundation/Rosetta.git
cd Rosetta
# compile, install. You may check more compilation options by checking `rosetta.sh --help`
./rosetta.sh compile --enable-protocol-mpc-securenn; ./rosetta.sh install
```

Before running your program, you should configure with your network topology so that a distributed network can be established for parties to communicate with each other.

<img src='doc/_static/figs/deployment.png'  width = "667" height = "400" align="middle"/>

You could use an example to check everything runs OK. Please refer to [Deployment Guide](doc/DEPLOYMENT.md) for the detailed steps of installation, configuration and deployment of Rosetta.

## Usage

The following is a toy [example](example/tutorials/code/rosetta_demo.py) for matrix multiplication using Rosetta.

In this example, we assume that three guys want to get the product of their private matrix, while do not want others to know what they hold. For brevity, we call them P0, P1 and P2.

With Rosetta, each of them can run the following script, from which you can see that only a small amount of codes are needed besides the native TensorFlow lines.

```python
#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# You can activate a backend protocol, here we use SecureNN
rtt.activate("SecureNN")

# Get private data from every party
matrix_a = tf.Variable(rtt.private_console_input(0, shape=(3, 2)))
matrix_b = tf.Variable(rtt.private_console_input(1, shape=(2, 1)))
matrix_c = tf.Variable(rtt.private_console_input(2, shape=(1, 4)))

# Just use the native tf.matmul operation.
cipher_result = tf.matmul(tf.matmul(matrix_a, matrix_b), matrix_c)

# Start execution
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    # Take a glance at the ciphertext
    cipher_result = sess.run(cipher_result)
    print('local ciphertext result:', cipher_result)
    # Set only party a and c can get plain result
    a_and_c_can_get_plain = 0b101 
    # Get the result of Rosetta matmul
    print('plaintext matmul result:', sess.run(rtt.SecureReveal(cipher_result, a_and_c_can_get_plain)))
```

To run this jointly, after configuring networks, the three guys can run the following command-line respectively:

```bash
python rosetta_demo.py --party_id=0
```

,

```bash
python rosetta_demo.py --party_id=1
```

and

```bash
python rosetta_demo.py --party_id=2
```

Then each party will be prompted to input his private matrix, for example P0 may have:

> [2020-07-29 20:10:49.070] [info] Rosetta: Protocol [SecureNN] backend initialization succeeded!
>
> please input the private data (float or integer, 6 items, separated by space): 2 3 1 7 6 2

Note that input from console like this is purely for pedagogical purpose in this toy example. See our [Doc](doc/API_DOC.md) for production-ready data APIs.

At the end, P0 and P2 will get the plaintext output while P1 dose not, just as required. Specifically, P0 and P2 may have:

> plaintext matmul result: [[b'8.000000' b'14.000000' b'18.000000' b'4.000000']
> [b'4.000000' b'7.000000' b'9.000000' b'2.000000']
> [b'24.000000' b'42.000000' b'54.000000' b'12.000000']]
>
> [2020-07-29 20:11:06.452] [info] Rosetta: Protocol [SecureNN] backend has been released.

while P1 has:
> plaintext matmul result: [[b'0.000000' b'0.000000' b'0.000000' b'0.000000']
> [b'0.000000' b'0.000000' b'0.000000' b'0.000000']
> [b'0.000000' b'0.000000' b'0.000000' b'0.000000']]
>
> [2020-07-29 20:11:06.452] [info] Rosetta: Protocol [SecureNN] backend has been released.

That's all, you can see Rosetta is so easy to use.

For more details, please check [Tutorials](doc/TUTORIALS.md) and [Examples](./example).

> Note: Currently Rosetta already supports 128-bit integer data type, which can be enabled by configuring the environment variable `export ROSETTA_MPC_128=ON`.

## Getting Started

To help you start with your first workable program with Rosetta easily, our [Tutorials](doc/TUTORIALS.md) will lead you to this fantastic world. In this detailed tutorials, we will assist you learn the basic concepts about Rosetta, then show you how to use the interfaces that we provide by easy-to-understand examples, and finally help you build a workable privacy-preserving machine learning model on real-world datasets.

Hopefully, this tutorial, and more other examples in [Examples](./example), will whet your appetite to dive in knowing more about this privacy-preserving framework.

## How Rosetta Works

Rosetta works by extending and hacking both Python frontend and the Operation Kernels in backend of TensorFlow. It decouples the development of TensorFlow-related components and privacy technology so that both developers from AI and cryptography can focus on what they are interested.

<img src='doc/_static/figs/architecture_detail_en.png' width = "700" height = "600" align="middle"/>

When running your Rosetta program, firstly the native TensorFlow data flow graph will be converted, and during this process the native Operations within the graph will be replaced with SecureOps.

<img src='doc/_static/figs/static_pass.png' width = "800" height = "400" align="middle"/>

And at the second stage, the backend kernels of operations, implemented with specific cryptographic protocol, will be called to carry out underlying secure computation.

<img src='doc/_static/figs/dynamic_pass.png' width = "800" height = "400" align="middle"/>

## Contributing to Rosetta

Rosetta is an open source project developed under the LPGLv3 license and maintained by [LatticeX Foundation](https://latticex.foundation/). Contributions from individuals and organizations are all welcome. Before beginning, please take a look at our [contributing guidelines](CONTRIBUTING.md). Our project adheres to [code of conduct](CODE_OF_CONDUCT.md). By participating in our community, you are expected to uphold this code. You could also open an issue by clicking [here](https://github.com/LatticeX-Foundation/Rosetta/issues/new).

## Documents List

* [Rosetta Tutorials](doc/TUTORIALS.md)

* [Rosetta Deployment Guide](doc/DEPLOYMENT.md)

* [Rosetta User API](doc/API_DOC.md)

* [Rosetta WhitePaper](doc/Rosetta_whitepaper.pdf)

* [Rosetta Glossary](doc/GLOSSARY.md)

* [Rosetta Release Notes](RELEASE.md)
  
* [FAQ](https://github.com/LatticeX-Foundation/Rosetta/wiki/FAQ)


## Citation

You can cite our work as folllows:
```latex 
    @misc{Rosetta,
      author = {Yuanfeng Chen, Gaofeng Huang, Junjie Shi, Xiang Xie, and Yilin Yan},
      title = {{Rosetta: A Privacy-Preserving Framework Based on TensorFlow}},
      howpublished = {\url{https://github.com/LatticeX-Foundation/Rosetta}},
      year={2020}
    }
```

## Reference

Check this [wiki page](https://github.com/LatticeX-Foundation/Rosetta/wiki/Reference) for the reference.

## Contact

You could reach us by [email](mailto:rosetta@latticex.foundation). **And you are welcome to join Rosetta community on [Slack](https://join.slack.com/t/latticexrosetta/shared_invite/zt-dum6j65d-MTxp~Bxq5OwouJW8zUKB1Q) to ask any questions and discuss on any interesting topics with other developers**.

## License

The Rosetta library is licensed under the [GNU Lesser General Public License v3.0](COPYING.LESSER).
