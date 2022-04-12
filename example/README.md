## Examples (tutorials)

These are examples to show you how to use Rosetta.

To help you understand the concepts easily, most of the examples have two parts, the first being the native Tensorflow version on centerized plaintext datasets, the second being the Rosetta version, which has the same functionality while running on secret-sharing 'ciphertext' datasets in a privacy-preserving way.

In order to run the examples smoothly, some dependencies need to be installed, as follows:

```sh
pip3 install matplotlib sklearn --user
```

<br/>


- [The tutorials of how to use Rosetta](./tutorials/README.md).



## Examples (zero knowledge proof)


Zero Knowledge Proof (ZKP) is a powerful cryptographic primitive. And Rosetta has integrated some cutting-edge ZKP protocols, such as [Mystique](https://eprint.iacr.org/2021/730). So in this tutorial, we will show you how to use this functionality in Rosetta.


**Compiling Rosetta with ZKP protocol enabled**

When You compile Rosetta, please make sure the `--enable-protocol-zk`  is enabled, as follows: 

```bash
./rosetta.sh compile --enable-protocol-zk
```

Note that we rely on submodule [emp-ot](https://github.com/emp-toolkit/emp-ot), [emp-tool](https://github.com/emp-toolkit/emp-tool) and [emp-zk](https://github.com/emp-toolkit/emp-zk).  So You may need to run command `git submodule init` and `git submodule update`  after cloning from Rosetta repository.

>  During this compilation process, Rosetta will first try to install these libraries in `emp-toolkit` mentioned above. So you may check whether these submodules' codes have been ready.



<br/>

- [Private Inference on Logistic Regression Model](./zkp_lr/README.md).
- [Private Inference on Deep Neural Network Model](./zkp_resnet/README.md).



<br/>


**Enjoy it.**
