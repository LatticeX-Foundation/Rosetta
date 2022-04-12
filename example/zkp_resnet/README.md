## Private Inference on Deep Neural Network Model

### Overview

Besides simple models like logistic regression, more sophisticated models, like ResNet, can also be transformed to their privacy-enhanced version with ZKP.

[ResNet](https://arxiv.org/abs/1512.03385) is a famous deep learning model, which performs very well in many image recognition competitions. As many other deep neural networks, it also consists of many layers of basic building blocks, which includes primitive operations like ***Convolution, Max Pooling, Sigmoid, SoftMax, Batch Normalization,*** and ***fully-connections***. 

This example code is in `example/zkp_resnet`.  

> Here we describe the whole process based on ResNet-101 model on CIFAR-10 dataset. For ResNet-50 model or using other datasets, which can be configured with command line options, please refer to the source code. 

  

### Demo Description

1. Get the plaintext ResNet model.

> You can directly download a good enough trained plaintext ResNet-50 and ResNet-101 model from https://pan.baidu.com/s/1vqNrHKZLai99nqOk0OCjKw   with the secret token `jkpa`. (If you have any problem getting this model, please contact us on Github). 

You can train the model by running the following command line in subdirectory `tf-train`.

```bash
# train resnet101 on cifar10
python3 main.py --phase train --dataset cifar10 --lr 0.0001 --res_n 101 \
--epoch 500 --batch_size 128 --train_size 50000 --test_size 10000
```

> You can change the `--dataset` or `--res_n` for other datasets or resnet type. For more details see `python3 main.py --help`.

And after this, you will get saved plaintext model checkpoints in its `checkpoint/ResNet101_cifar10` directory such as:


```
checkpoint
ResNet.model-55770.data-00000-of-00001
ResNet.model-55770.index
ResNet.model-55770.meta
```

2. Plaintext inference to verify the trained model

Before we run with ZKP protocol, we first run the native TF inference to get the plaintext prediction results to verify the accuracy of the trained model, and moreover, to prepare for later comparison with the revealed ZKP results.

First, we should copy the trained models (the whole directory `checkpoint`) to subdirectory `tf-predict`. 

And then we run the following command line to get the plaintext prediction result in local file `tf-preds-ResNet101_cifar10.csv`: 

```bash
python3 main.py --phase test --res_n 101 --dataset cifar10 --test_size 10000
```

The first line in this csv file may look like:

```
0.0000003133 0.0000014009 0.0000002701 0.9999711514 0.0000000138 0.0000267564 0.0000000021 0.0000000116 0.0000000114 0.0000000796
```

3. **Inference with ZKP**

Just as the last step, we also need to copy the trained models (the whole directory `checkpoint`) to subdirectory `rtt-predict`. 

We have tested on three cases. The first is that both the testing dataset and the model are private to Prover. The second is that the testing dataset is private to Prover while the model are public. The last is opposite to the second one, thus meaning the model are private to Prover while the testing dataset is public.

You can run all the three cases in local standalone mode just running the one-in-all script:


```bash
./run_batch_local_cifar10_101.sh
```

> If the program runs very slow or even exit early on your host machine, you may consider using a better  server with enough memory to run this task.

If we set the `test_size` as 1, we can get the secure inference result for the first test dataset in  `log/res/rtt-preds-ResNet101_cifar10-0.csv` :

```
0.0000000000 0.0000000000 0.0000000000 0.9999690000 0.0000000000 0.0000150000 0.0000000000 0.0000000000 0.0000000000 0.0000000000
```

So comparing with the plaintext result, it almost has no loss of accuracy.


