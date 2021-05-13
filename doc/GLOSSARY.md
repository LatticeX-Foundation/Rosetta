# Glossary

- [Glossary](#glossary)
  - [Overview](#overview)
  - [Terms and definitions](#terms-and-definitions)

----

## Overview

This document will maintain and continually update various terms and abbreviations appeared in the Rosetta framework.

## Terms and definitions

- **Secure Multi-Party Computation (MPC)**
A field of cryptography. Multiple participants jointly compute functions, each participant will get the result at the end of the protocol, while no additional information beyond the result will be leaked during the entire process.

- **Zero-Knowledge Proof (ZKP)**
A field of cryptography. It involves in two participants: prover and verifier. The prover could convince the verifier without leaking additional information.

- **Homomorphic Encryption (HE)**
A field of cryptography. The message is encrypted to ciphertext, anyone could perform computations on ciphertexts, and the operations could map to the plaintexts.

- **Federated Learning (FL)**
An artificial intelligence algorithm. Multiple participants jointly train models, the intermediate trained weight gradients are shared by all the parties to finally get the whole model.

- **Trusted Execution Environment (TEE)**
It is a secure area of a main processor. It guarantees code and data loaded inside to be protected with respect to confidentiality and integrity.
- **Secret Sharing（SS）**
A method to implement secure multi-party computation. The data is divided to different shares. Each participant owns one of the shares, and one the share is uniformly random and independent of the data.

- **Pass**
It is originally a part of the compiler. In Rosetta it is used to replace and optimize native TensorFlow operations with the privacy operations.

- **Static Pass**
It is a specific Pass. In Rosetta, it replaces operations with privacy operations before the directed graph is executed.

- **Dynamic Pass**
It is a specific Pass. In Rosetta, it replaces operations with privacy operations when executing the directed graph.
