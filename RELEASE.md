
# Release Notes

- [Release Notes](#release-notes)
  - [Introduction](#introduction)
  - [Rosetta v1.0.0](#rosetta-v100)
  - [Rosetta v0.3.0](#rosetta-v030)
  - [Rosetta v0.2.1](#rosetta-v021)
  - [Rosetta v0.2.0](#rosetta-v020)
  - [Rosetta v0.1.1](#rosetta-v011)
    - [Features](#features)
  - [Rosetta v0.1.0](#rosetta-v010)
    - [Features](#features-1)
    - [Supported Platforms](#supported-platforms)
    - [Known Problems](#known-problems)
  - [Additional Information](#additional-information)

----

## Introduction

This document will maintain and continually update the release notes of each version of Rosetta. If you have questions or comments, please contact us via rosetta@latticex.foundation.

## Rosetta v1.0.0

1. Support multi-tasking concurrency, that mean different privacy protocols can be executed simultaneously.
   
2. Nodes can be configured with roles, supporting calculation node, data node and result node, and data nodes can be configured in any number according to requirements.
   
3. Refactoring code to make the code highly cohesive and low coupled.
   
4. Added SecureExp, SecureSqrt, SecureRsqrt, SecureReciprocaldiv secure operations.
   
5. Adding audit logs, which are turned off by default and can be turned on according to business scenarios.
   
6. Efficient third-party io modules can be integrated into Rosetta by implementing specific interfaces.
   
7. Optimize back-end unit tests to provide more friendly test cases.
   
8. Some known bugs are fixed.

## Rosetta v0.3.0

1. Added `PrivateTextLineDataset`, `PrivateInput` secure operations.

2. Added `SecureLogicalAnd`, `SecureLogicalOr`,  `SecureLogicalXor`, `SecureLogicalNot` secure operations.

3. Speedup some backend operations.

4. Uses related python classes such as PrivateTextLineDataset and iterators to load large data sets, thereby reducing memory usage.

5. Some known bugs are fixed.

## Rosetta v0.2.1

1. Support 128-bit integer data type, which enables big integer and high precision float-point operations.

2. Refactor python directories of dynamic libraries and use `ROSETTA_MPC_128=ON` to enable 128-bit privacy computation.

## Rosetta v0.2.0

1. Refactor the whole architecture so that each layer has a clearer boundary. Especially, an abstract protocol layer is added to decouple cryptographic protocols and TensorFlow Ops, so that TensorFlow does not depend on specific protocols and vise verse.

2. Support custom data types by mainly utilizing `tf.string`. This will enable us to have arbitrary length and format of internal ciphertext.

3. Some APIs, such as `activate`, are added and refined to support better control on selection and configuration of backend protocol. In this version, you can configure IP ports, host addresses, save mode flag and protocol precision as JSON string by calling `activate` in your program.

4. Some known bugs are fixed.

## Rosetta v0.1.1

### Features

1. Binary installation of TensorFlow is supported.

## Rosetta v0.1.0

### Features

1. Secure Multi-Party Computation (MPC) is supported, the underlying protocol is [SecureNN](https://eprint.iacr.org/2018/442.pdf). This is a $3$-party protocol, which is secure in the semi-honest model with honest majority.

2. Developers could transfer traditional TensorFlow codes into a privacy-preserving manner with minimal changes (import latticex.rosetta).

3. Static Pass is supported to replace TensorFlow native operations with MPC operations before the execution of the graph.

4. Dynamic Pass is supported to replace TensorFlow native operations with MPC operations when the graph is executed.

5. The following MPC operations and related gradients are supported.

    |  MPC OPs     |    MPC Gradient OPs    |
    | --------------- | -------------- |
    |MpcAdd |MpcAddGrad|
    |MpcSub |MpcSubGrad|
    |MpcMul |MpcMulGrad|
    |MpcDiv |MpcDivGrad|
    |MpcTrueDiv |MpcTrueDivGrad|
    |MpcRealDiv |MpcRealDivGrad|
    |MpcMatMul |MpcMatMulGrad|
    |MpcSigmoid |MpcSigmoidGrad|
    |MpcLog |MpcLogGrad|
    |MpcLog1p |MpcLog1pGrad|
    |MpcPow |MpcPowGrad|
    |MpcMax |MpcMaxGrad|
    |MpcMean |MpcMeanGrad|
    |MpcRelu |MpcReluGrad|
    |MpcEqual |-|
    |MpcLess |-|
    |MpcGreater |-|
    |MpcLessEqual |-|
    |MpcGreaterEqual |-|
    |MpcSaveV2 |-|
    |MpcApplyGradientDescentOp |-|

6. Support the combination of supported MPC operations to implement arbitrary models, such as: linear regression model, logistic regression model, etc.

7. Support for specifying the type (plaintext or ciphertext) of model to save;

8. Support for specifying where to save the model (Party0\Party1\Party2\all Parties);

### Supported Platforms

Rosetta has only been extensively tested on Intel X64 machines running Ubuntu 18.04.

### Known Problems

This section contains all known problems with the Rosetta system, listed by component. As new problems are discovered, they will be added to these sections.

## Additional Information

More details of Rosetta could be found in
the [documentation fold](doc/). If you have any questions or comments about Rosetta, please feel free to contact us via rosetta@latticex.foundation.
