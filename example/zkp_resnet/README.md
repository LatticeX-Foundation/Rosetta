[toc]

# Private Inference with Zero Knowledge Proof on Rosetta

Zero Knowledge Proof (ZKP) is a powerful cryptographic primitive. And Rosetta has integrated some cutting-edge ZKP protocols, such as [Mystique](https://eprint.iacr.org/2021/730).  So in this tutorial, we will show you how to use this functionality in Rosetta.

> For historical reason, the name for the ZKP protocol in Rosetta is called `mystique`.

## Compiling Rosetta with ZKP protocol enabled

When You compile Rosetta, please make sure the `--enable-protocol-zk`  is enabled, as follows: 

```
./rosetta.sh compile --enable-protocol-zk --enable-tests
```

Note that we rely on submodule `emp-ot` [[code](https://github.com/emp-toolkit/emp-ot)], `emp-tool`[[code](https://github.com/emp-toolkit/emp-tool)] and `emp-zk` [[code](https://github.com/emp-toolkit/emp-zk)].  So You may need to run command `git submodule init` and `git submodule update`  after cloning from Rosetta repository.

>  During this compilation process, Rosetta will first try to install these libraries in `emp-toolkit` mentioned above. So you may check whether these submodules' codes have been ready.



## Examples 

### Private  Inference on Logistic Regression Model

#### Overview

This example code is in `example/zkp_lr`.  It is based on the MNIST dataset, and  in this sample demo we just train a logistic regression model, a simple binary classifier, to learn whether one hand-written digital image should be labeled as number `0`.  

**It is highly recommended that you run this simple demo to verify that you have installed the Rosetta with ZKP OK.**

You can run all the examples just with `./run.sh`. And you will see the logs in `log` subdirectory.

 And finally, you can see the mean relative errors between the plaintext TensorFlow inference results and ZKP-powered private inference results on MNIST test dataset. 

For example,  you may see:

   >run tf logistic regression   train mnist. ...
   >
   >run tf logistic regression predict mnist. ...
   >
   >run zk logistic regression predict mnist. ...
   >
   >run zk logistic regression predict mnist. model loaded as public ...
   >
   >run zk logistic regression predict mnist. input is public ...
   >np.mean(prediction probability relative_error): 3.225990394471199e-05
   >error_in_bits: 16.45493854813948



#### Demo Description

We first train a plaintext LR model with the script `tf-logistic_regression_train.py`, and this model will be saved as a checkpoint in subdirectory `./log/ckp0`.  The log file for this script is saved as `./log/mnist_logistic_regression_train.log`. 

And then we test on the test dataset directly with this learned model in another native TF script `tf-logistic_regression_predict.py`, and the log file for this script will be `./log/mnist_logistic_regression_predict.log`.  You may see the testing metrics as follows:

> {
>
>  "tag": "tf",
>
>  "score_auc": 0.9841388318716179,
>
>  "score_ks": 0.9522298936368726,
>
>  "threshold_opt": 0.6165876159789364,
>
>  "score_accuracy": 0.9921875,
>
>  "score_precision": 0.9957081545064378,
>
>  "score_recall": 0.9957081545064378,
>
>  "score_f1": 0.9957081545064378
>
> }

This script will also output the detailed predication value for each testing sample in file `./log/preds_tf_mnist.csv`, and this is the baseline for comparing later with the private-preserving result to check the precision of ZKP.



And then, we can try our ZKP protocol to secure this inference process with Rosetta. 

First of all, we evaluate on the case where both the testing data and the trained model parameters are kept private for Prover. The following two lines in `run.sh` is for this case.

```bash
python3 rtt-logistic_regression_predict.py --party_id=1 >log/mnist_logistic_regression_predict-1.log 2>&1 &
python3 rtt-logistic_regression_predict.py --party_id=0 >log/mnist_logistic_regression_predict-0.log 2>&1
```

You can see from the script `rtt-logistic_regression_predict.py` that we just `private_input` the testing sample data and restore the plaintext model from checkpoint for Prover.

To check the result, we can reveal and get the testing metrics as follows in `log/mnist_logistic_regression_predict-0.log`:

>{
>
> "tag": "zk",
>
> "score_auc": 0.9841388318716179,
>
> "score_ks": 0.9522298936368726,
>
> "threshold_opt": 0.616577,
>
> "score_accuracy": 0.9921875,
>
> "score_precision": 0.9957081545064378,
>
> "score_recall": 0.9957081545064378,
>
> "score_f1": 0.9957081545064378
>
>}

For later comparison, we also reveal the detailed prediction value in file `log/preds_zk_mnist.csv`.



We also evaluate on the cases when the testing dataset can be public and when the model parameters can be public.  This can be done by running:

```bash
echo "run zk logistic regression predict mnist. Model loaded as public ..."
kill_prog
python3 rtt-logistic_regression_predict.py --party_id=1 --model_public --cfgfile=CONFIG-public.json >log/mnist_logistic_regression_predict-model_public-1.log 2>&1 &
python3 rtt-logistic_regression_predict.py --party_id=0 --model_public --cfgfile=CONFIG-public.json >log/mnist_logistic_regression_predict-model_public-0.log 2>&1

echo "run zk logistic regression predict mnist. Test dataset as public ..."
kill_prog
python3 rtt-logistic_regression_predict.py --party_id=1 --input_public >log/mnist_logistic_regression_predict-input_public-1.log 2>&1 &
python3 rtt-logistic_regression_predict.py --party_id=0 --input_public >log/mnist_logistic_regression_predict-input_public-0.log 2>&1
```

The results are similar.



As said above, we can also compare the mean prediction error for all test dataset. By running `python relative_error.py` in the end, we can get something like:

> np.mean(prediction probability relative_error): 2.0313663485231745e-05

This means that comparing to native TensorFlow inference, The ZKP-backed secure inference has virtually no loss of accuracy.



### Private Inference on Deep Neural Network Model

#### Overview

Besides simple models like logistic regression, more sophisticated models, like ResNet, can also be transformed to their privacy-enhanced version with ZKP.

[ResNet](https://arxiv.org/abs/1512.03385) is a famous deep learning model, which performs very well in many image recognition competitions. As many other deep neural networks, it also consists of many layers of basic building blocks, which includes primitive operations like ***Convolution, Max Pooling, Sigmoid, SoftMax, Batch Normalization,*** and ***fully-connections***. 

This example code is in `example/zkp_resnet`.  

> Here we describe  the whole process based on ResNet-101 model on CIFAR-10 dataset. For ResNet-50 model or using other datasets, which can be configured with command line options, please refer to the source code. 

  

#### Demo Description

1. Get the plaintext ResNet model.

   > You can directly download a good enough trained plaintext ResNet-50 and ResNet-101 model from https://pan.baidu.com/s/1vqNrHKZLai99nqOk0OCjKw   with the secret token `jkpa` .(If you have any problem getting this model, please contact us on Github). 

   You can train the model by running the following command line in subdirectory `tf-train`.

   ```bash
   python3 main.py --phase train --dataset cifar10 --lr 0.0001 --res_n 101 --epoch 500 --batch_size 128 --train_size 50000 --test_size 10000
   ```

   And after this, you will get saved plaintext model checkpoints in its `checkpoint/ResNet101_cifar10` directory such as:

   > checkpoint
   >
   > ResNet.model-55770.data-00000-of-00001
   >
   > ResNet.model-55770.index
   >
   > ResNet.model-55770.meta

2. Plaintext inference to verify the trained model

   Before we run with ZKP protocol, we first run the native TF inference to get the plaintext prediction results to verify the accuracy of the trained model, and moreover, to prepare for later comparison with the revealed ZKP results.

   First, we should copy the trained models (the whole directory `checkpoint`) to subdirectory `tf-predict`. 

   And then we run the following command line to get the plaintext prediction result in local file `tf-preds-ResNet101_cifar10.csv`: 

   ```bash
   python3 main.py --phase test --res_n 101 --dataset cifar10 --test_size 10000
   ```

   The first line in this csv file may look like:

   > 0.0000003133 0.0000014009 0.0000002701 0.9999711514 0.0000000138 0.0000267564 0.0000000021 0.0000000116 0.0000000114 0.0000000796

3. **Inference with ZKP**

   Just as the last step, we also need to copy the trained models (the whole directory `checkpoint`) to subdirectory `rtt-predict`. 

   We have tested on three cases. The first is that both the testing dataset and the model are private to Prover. The second is that the testing dataset is private to Prover while the model are public. The last is opposite to the second one, thus meaning the model are private to Prover while the testing dataset is public.

   You can run all the three cases in local standalone mode just running the one-in-all script:

   ```bash
   ./run_batch_local_cifar10_101.sh
   ```

   > If the program runs very slow or even exit early on your host machine, you may consider using a better  server with enough memory to run this task.

   If we set the `test_size` as 1, we can get the secure inference result for the first test dataset in  `log/res/rtt-preds-ResNet101_cifar10-0.csv` :

   > 0.0000000000 0.0000000000 0.0000000000 0.9999690000 0.0000000000 0.0000150000 0.0000000000 0.0000000000 0.0000000000 0.0000000000

   So comparing with the plaintext result, it almost has no loss of accuracy.

   
