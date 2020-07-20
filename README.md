![LOGO](https://mmbiz.qpic.cn/mmbiz_png/dV0Pt26LydDKo3HFIeH8afhT8XCmZibWhmj4vuVyuyGQrb0U4vIicibd5xjQKPOib7ibhFRWia9mdbz8uyricY9ZbDgXg/640)

[![github license](https://img.shields.io/badge/license-LGPLv3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

--------------------------------------------------------------------------------

## Overview

Rosetta is a privacy-preserving framework based on [TensorFlow](https://www.tensorflow.org). It integrates with mainstream privacy-preserving computation technologies, including cryptography, federated learning and trusted execution environment. Rosetta aims to provide privacy-preserving solutions for artificial intelligence without requiring expertise in cryptography, federated learning and trusted execution environment. Rosetta reuses the APIs of TensorFlow and allows to transfer traditional TensorFlow codes into a privacy-preserving manner with minimal changes. E.g., just add the following line.

```python
import latticex.rosetta
```
The current version integrates the secure multi-party computation protocols for 3 parties. The underlying protocol is [SecureNN](https://eprint.iacr.org/2018/442.pdf). It is secure in the semi-honest model with honest majority.

## Installation

For now, Rosetta runs on Ubuntu 18.04, and is based on TensorFlow 1.14 with CPUs. Windows is not currently supported. Install Rosetta as follows.

Please check that your local system meets our base environment requirement. 

Install the native TensorFlow with the following codes. Note that you could also install it from source code, check [here](doc/TENSORFLOW_INSTALL.md) for details.

```bash
# install tensorflow
pip3 install tensorflow==1.14.0
```

Build and install Rosetta with our all-in-one script as follows.

```bash
# clone rosetta git repository
git clone https://github.com/LatticeX-Foundation/Rosetta.git
cd Rosetta
# compile, install and run test cases
bash compile_and_test_all.sh
```

You could use an example to check everything runs OK. Please refer to [Deployment Guide](doc/DEPLOYMENT.md) for the detailed steps of configuration, installation and deployment of Rosetta.

## Usage

The following is a simple example for matrix multiplication using Rosetta.


```python
#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# You can activate a backend protocol, here use SecureNN
rtt.activate("SecureNN")

# Get private data from Alice (input x), Bob (input y)
x = tf.Variable(rtt.private_console_input(0, shape=(2, 3)))
y = tf.Variable(rtt.private_console_input(1, shape=(2, 3)))

# Define matmul operation
res = tf.matmul(x, y)

# Start execution
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    res = sess.run(res)

    # Get the result of Rosetta matmul
    print('matmul:', sess.run(rtt.SecureReveal(res)))
```
For more details, please check [Tutorials](doc/TUTORIALS.md) and [Examples](./example).

## Getting Started

To help you start with your first workable program with Rosetta easily, our [Tutorials](doc/TUTORIALS.md) will lead you to this fantastic world. In this detailed tutorials, we will assist you learn the basic concepts about Rosetta, then show you how to use the interfaces that we provide by easy-to-understand examples, and finally teach you to build a workable privacy-preserving machine learning model on real datasets.

Hopefully, this tutorial, and more other examples in [Examples](./example), will whet your appetite to dive in knowing more about this privacy-preserving framework.


## Contributing to Rosetta

Rosetta is an open source project developed under the LPGLv3 license and maintained by [LatticeX Foundation](https://latticex.foundation/). Contributions from individuals and organizations are all welcome. Before beginning, please take a look at our [contributing guidelines](CONTRIBUTING.md). You could also open an issue by clicking [here](https://github.com/LatticeX-Foundation/Rosetta/issues/new).

## Documents List

* [Rosetta Tutorials](doc/TUTORIALS.md)

* [Rosetta Deployment Guide](doc/DEPLOYMENT.md)

* [Rosetta User API](doc/API_DOC.md)

* [Rosetta WhitePaper](doc/Rosetta_whitepaper.pdf)

* [Rosetta Glossary](doc/GLOSSARY.md)

* [Rosetta Release Notes](RELEASE.md)


## Contact

You could reach us by [email](mailto:rosetta@latticex.foundation), and join Rosetta community on [Slack](https://latticexrosetta.slack.com).


## License

The Rosetta library is licensed under the [GNU Lesser General Public License v3.0](COPYING.LESSER).
