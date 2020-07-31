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
For more details, please check [Tutorials](doc/TUTORIALS.md) and [Examples](./example).

> Note: Currently Rosetta already supports 128-bit integer data type, which can be enabled by configuring the environment variable `export ROSETTA_MPC_128=ON`.

## Getting Started

To help you start with your first workable program with Rosetta easily, our [Tutorials](doc/TUTORIALS.md) will lead you to this fantastic world. In this detailed tutorials, we will assist you learn the basic concepts about Rosetta, then show you how to use the interfaces that we provide by easy-to-understand examples, and finally help you build a workable privacy-preserving machine learning model on real-world datasets.

Hopefully, this tutorial, and more other examples in [Examples](./example), will whet your appetite to dive in knowing more about this privacy-preserving framework.


## Contributing to Rosetta

Rosetta is an open source project developed under the LPGLv3 license and maintained by [LatticeX Foundation](https://latticex.foundation/). Contributions from individuals and organizations are all welcome. Before beginning, please take a look at our [contributing guidelines](CONTRIBUTING.md). Our project adheres to [code of conduct](CODE_OF_CONDUCT.md). By participating in our community, you are expected to uphold this code. You could also open an issue by clicking [here](https://github.com/LatticeX-Foundation/Rosetta/issues/new).

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
