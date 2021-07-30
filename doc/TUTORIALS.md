
- [Overview](#overview)
- [Installation and Deployment](#installation-and-deployment)
- [Quick Start](#quick-start)
- [Secure Multi-Party Computation](#secure-multi-party-computation)
  - [Millionaires' Problem](#millionaires-problem)
    - [TensorFlow Version](#tensorflow-version)
    - [Rosetta Version](#rosetta-version)
- [Privacy-Preserving Machine Learning](#privacy-preserving-machine-learning)
  - [Linear Regression](#linear-regression)
    - [TensorFlow Version Linear Regression](#tensorflow-version-linear-regression)
    - [Rosetta Basic Version](#rosetta-basic-version)
    - [Rosetta Version-with Reveal](#rosetta-version-with-reveal)
    - [Comparison and Evaluation 1](#comparison-and-evaluation-1)
    - [Comparison and Evaluation 2](#comparison-and-evaluation-2)
    - [Model Saving](#model-saving)
    - [Model Loading and Prediction](#model-loading-and-prediction)
  - [Logistic Regression](#logistic-regression)
  - [Support big data sets](#support-big-data-sets)
  - [Support stop Rosetta execution at any time](#support-stop-rosetta-execution-at-any-time)
- [Privacy-Preserving Deep Learning](#privacy-preserving-deep-learning)
  - [MLP Neural Network](#mlp-neural-network)
    - [TensorFlow Version MLP](#tensorflow-version-mlp)
    - [Rosetta Version MLP](#rosetta-version-mlp)
- [Support multitasking concurrency](#support-multitasking-concurrency)
- [Conclusion](#conclusion)
- [Additional Notes](#additional-notes)
  - [Dataset Description](#dataset-description)

## Overview

## Installation and Deployment

If you have not set up a `Rosetta` environment yet, please refer to [Deployment Document](./DEPLOYMENT.md).

In order to simplify the description of this tutorial, the examples below are based on `single machine with multiple nodes` mode, please refer to `Deployment Documentation` for deployment in this way.

## Quick Start

All of the following content is based on the previous installation and deployment you have completed. For saving time, please make sure that your environment is already `OK`.

Unless otherwise noted, all commands are run in the path of `example/tutorials/code/`.

<br/>

Now, let us enter this exciting moment together: how to use `Rosetta` in the easiest way?

The steps are very simple. Whenever you want to use `Rosetta` (`Python` script file), just import our `Rosetta` package, as follows:

```python
import latticex.rosetta as rtt
```

<font style = "color: green"> Note: </font>

`rtt` is short for `Rosetta`, just like `tf` for `TensorFlow`, `np` for `numpy`, `pd` for `pandas` . Let's just view this as a convention.

<br/>

You can run directly in the same terminal, as follows:

```sh
./tutorials.sh rtt quickstart
```

Or, you can run the following program on three different terminals (just to simulate the scenario that three different parties are running on their own private data on their own machine):

```sh
# node 0
python3 rtt-quickstart.py --party_id=0
```

```sh
# node 1
python3 rtt-quickstart.py --party_id=1
```

```sh
# node 2
python3 rtt-quickstart.py --party_id=2
```

If you see `DONE!` in the output, you have accomplished the first goal.

> `--party_id` This is a command line option that specifies which role the current running script is playing.
> In the following, in order to describe examples concisely, we will just directly uses `./tutorials.sh` to show that we are running the three steps above.

<br/>

The following tutorial is just as easy as this `Quick Start`. Let's continue.

## Secure Multi-Party Computation

Let's assume that there are `two` `honest` rich millionaires who are discussing on `who has more wealth`, while neither of them wants to tell the other one the specific number on their bank account. What can I do? I can't help it, but `Rosetta` can help you. Now, let's try to solve this problem.

### Millionaires' Problem

The problem we have just proposed is a famous example in MPC, called [Yao's Millionaires' Problem](https://en.wikipedia.org/wiki/Yao%27s_Millionaires%27_Problem).

Let‘s be a little more specific. In the following example, we will introduce two millionaires, one called `Alice` and the other called `Bob`, have `$2000001` and `$2000000` respectively. You see, the difference between the two people's wealth is just `one` dollars.

#### TensorFlow Version

For comparison，Let's just solve this problem without taking their privacy into consideration at first.

This is trivial, every child can do this in less than one second. But in order to compare with `Rosetta`, here we write a toy program in `TensorFlow` style:

The first step is to import the package:

```py
import tensorflow as tf
```

The second step is to set how much wealth each has:

```py
Alice = tf.Variable(2000001)
Bob = tf.Variable(2000000)
```

The third step is to call `session.run` and check the results:

```py
res = tf.greater(Alice, Bob)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    ret = sess.run(res)
    print("ret:", ret)  # ret: True
```

Refer to [tf-millionaire.py](../example/tutorials/code/tf-millionaire.py) for the full version.

And now, we run this program as follows:

```sh
./tutorials.sh tf millionaire
```

The output will show:

```log
ret: True
```

The result shows that `Alice` has more wealth than `Bob`.

It's intuitive, I won't go into details.

#### Rosetta Version

The above is a artificial one. Now let's see how to use `Rosetta` to solve the original problem of millionaires without leaking their privacy. Let's begin, and you will see it is very simple!

The first step is to import the `Rosetta` package.

```py
import latticex.rosetta as rtt
import tensorflow as tf
```

The second step is activate protocol.

```py
rtt.activate("SecureNN")
```

The third step is to set how much wealth each has privately.

We use the built-in `rtt.private_console_input` to get the private data entered by the user in the terminal.

```py
Alice = tf.Variable(rtt.private_console_input(0))
Bob = tf.Variable(rtt.private_console_input(1))
```

The forth step is exactly the same as `TensorFlow`.

```py
res = tf.greater(Alice, Bob)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    res = sess.run(res)
    print('res:', res)  # res: b'\x90\xa3\xff\x14\x87f\x95\xc3#'
```

The above output of `res` is a `sharing` value in [Secret-Sharing scheme](GLOSSARY.md).

> The `sharing` value is a random number essentially. an original true value x, is randomly split into two 64-bit values ​​x0, x1 (with x = x0 + x1), which are held by P0 and P1, respectively.

How do we know the plaintext value? We provide a `reveal` interface to get the plain text value for convenience in debugging and testing. Just add it after the forth step:

```py
with tf.Session() as sess:
    # ...
    ret = rtt.SecureReveal(res)
    print('ret:', sess.run(ret))  # ret: b'1.000000'
```

For the complete program of console version, refer to [rtt-millionaire-console.py](../example/tutorials/code/rtt-millionaire-console.py).

For the complete program of script version, refer to  [rtt-millionaire.py](../example/tutorials/code/rtt-millionaire.py).

Run this program as follows:

```sh
./tutorials.sh rtt millionaire
```

The output is as follows:

```log
ret: 1.0
```

The result indicates that `Alice` has more wealth than `Bob`.

<br/>

> For a description of all operators supported by `Rosetta` including `SecureReveal`, please refer to our [API Document](./API_DOC.md).

<br/>

Of course, `Rosetta` can not only be used to solve simple problems  like `Millionaire's Problem`. Next, let’s see how we can tackle real problems in `Machine Learning` with the help of `Rosetta` framework.

## Privacy-Preserving Machine Learning

We will talk about the combination of `Privacy-Preserving` and`Machine Learning (ML)` in this part. Let's start with the simplest machine learning model: `Linear Regression`.

### Linear Regression

This section introduces how to use `Rosetta` to perform a complete `Linear Regression` task, including `data processing`, `training and model saving`, `model loading and prediction` and `evaluation`.

Before using `Rosetta` for machine learning, in order to compare with `Rosetta`-backed MPC version, let's see how we do the same task in native `TensorFlow` style without the concern of data privacy.

#### TensorFlow Version Linear Regression

Here is a simple `Linear Regression` with `TensorFlow`.

- Import necessary packages, set training parameters, etc.

```py
import math
import os
import csv
import tensorflow as tf
import numpy as np
import pandas as pd
from util import read_dataset

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

EPOCHES = 10
BATCH_SIZE = 16
learning_rate = 0.0002
```

- Load dataset

Please refer to the appendix at the end of this tutorial.

We highlight codes that are different from `Rosetta`, and we will focus more on them later.

```py
# real data
# ######################################## difference from rosettta
file_x = '../dsets/ALL/reg_train_x.csv'
file_y = '../dsets/ALL/reg_train_y.csv'
real_X, real_Y = pd.read_csv(file_x).to_numpy(), pd.read_csv(file_y).to_numpy()
# ######################################## difference from rosettta
DIM_NUM = real_X.shape[1]
```

- Write a Linear Regression model.

We will not go into details here, and only present the code below. For details, please refer to `TensorFlow™` official website about writing machine learning models.

```py
X = tf.placeholder(tf.float32, [None, DIM_NUM])
Y = tf.placeholder(tf.float32, [None, 1])

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1]))
b = tf.Variable(tf.zeros([1]))

# predict
pred_Y = tf.matmul(X, W) + b

# loss
loss = tf.square(Y - pred_Y)
loss = tf.reduce_mean(loss)

# optimizer
train = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)

init = tf.global_variables_initializer()
```

- Model training.

```py
with tf.Session() as sess:
    sess.run(init)
    xW, xb = sess.run([W, b])
    print("init weight:{} \nbias:{}".format(xW, xb))

    # train
    BATCHES = math.ceil(len(real_X) / BATCH_SIZE)
    for e in range(EPOCHES):
        for i in range(BATCHES):
            bX = real_X[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            bY = real_Y[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            sess.run(train, feed_dict={X: bX, Y: bY})

            j = e * BATCHES + i
            if j % 50 == 0 or (j == EPOCHES * BATCHES - 1 and j % 50 != 0):
                xW, xb = sess.run([W, b])
                print("I,E,B:{:0>4d},{:0>4d},{:0>4d} weight:{} \nbias:{}".format(
                    j, e, i, xW, xb))

    # predict
    Y_pred = sess.run(pred_Y, feed_dict={X: real_X, Y: real_Y})
    print("Y_pred:", Y_pred)
```

For complete code reference Please refer to [tf-linear_regression.py](../example/tutorials/code/tf-linear_regression.py) for the complete source code.

And then, we run this program as follows:

```sh
./tutorials.sh tf linear_regression
```

The output is as follows:

```log
Y_pred: [[4.8402567]
 [5.297159 ]
 [5.81963  ]
 ...
 [4.9908857]
 [5.8464894]
 [6.157756 ]]
```

#### Rosetta Basic Version

As mentioned above, if you have an existing model training script (`.py`) written with `TensorFlow`, then all you need to do is import the following packages on the first line of this script file:

```python
import latticex.rosetta as rtt
```

**Yes, it's that simple!** You don't need to modify any already written code. Even if you don’t have any knowledge of cryptography, you can use it easily. The only difference is about the how to set your private data.

- Activate protocol

```py
rtt.rtt.activate("SecureNN")
```

> Note: The protocol must be activated before any `MPC` related `API` can be used.

- Load dataset

Please refers to the appendix at the end of this article for the dataset description.

We have highlighted the spots that are different from `TensorFlow`. In contrast to the native `TensorFlow` version without data privacy. Except for the importing of the `Rosetta` package, only these several lines are different.

`Rosetta` provides a class, `PrivateDataset`, specifically for handling private data sets. Check the relevant source code for details.

```py
# real data
# ######################################## difference from tensorflow
file_x = '../dsets/P' + str(rtt.mpc_player.id) + "/reg_train_x.csv"
file_y = '../dsets/P' + str(rtt.mpc_player.id) + "/reg_train_y.csv"
real_X, real_Y = rtt.PrivateDataset(data_owner=(
    0, 1), label_owner=1).load_data(file_x, file_y, header=None)
# ######################################## difference from tensorflow
DIM_NUM = real_X.shape[1]
```

Please refer to [rtt-linear_regression.py](../example/tutorials/code/rtt-linear_regression.py) for the complete source code.

<br/>

OK. Now let's briefly summarize the difference from the `TensorFlow` version:

- Importing `Rosetta` package.
- Activate protocol.
- Loading data sets.

<br/>

Now, we have written the complete code, then how do we run it?

Do you still remember the above `Millionaires' Problem` example that you have learned? There is no difference in running them.

Just run it as follows:

```sh
./tutorials.sh rtt linear_regression
```

The output is as follows:

```log
Y_pred: [[b'\x9f\xf5\n\xc2\x81\x06\x00\x00#']
 [b'g6j\x7fq\x0f\x00\x00#']
 [b'\x95\xfc\x06\x1cA}\x00\x00#']
 ...
 [b'\x19\x02\xd5\xfd\xf1c\x00\x00#']
 [b'\xe1\xd5\x16pGz\x00\x00#']
 [b'}\xfe8\xd3,\x91\xff\xff#']]
```

Don't be panic, what you're seeing are `random` values in [`Secret sharing` scheme](GLOSSARY.md), rather than plaintext values, because Rosetta is protecting the privacy of your own data right now.

#### Rosetta Version-with Reveal

The `sharing value` output in the previous section is not human-readable. For testing, debugging, comparison with plain text ans so on, we provide a `reveal` interface to get the plain text value.

<font style = "color: red"> Reminder:please be cautious when using this reveal interface in a production environment. </font>

Let's modify the previous (basic version) program with addition of `reveal`, and see what effect it has. The new program is as follows:

```py
# ########### for test, reveal
reveal_W = rtt.SecureReveal(W)
reveal_b = rtt.SecureReveal(b)
reveal_Y = rtt.SecureReveal(pred_Y)
# ########### for test, reveal

with tf.Session() as sess:
    sess.run(init)
    rW, rb = sess.run([reveal_W, reveal_b])
    print("init weight:{} \nbias:{}".format(rW, rb))

    # train
    BATCHES = math.ceil(len(real_X) / BATCH_SIZE)
    for e in range(EPOCHES):
        for i in range(BATCHES):
            bX = real_X[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            bY = real_Y[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            sess.run(train, feed_dict={X: bX, Y: bY})

            j = e * BATCHES + i
            if j % 50 == 0 or (j == EPOCHES * BATCHES - 1 and j % 50 != 0):
                rW, rb = sess.run([reveal_W, reveal_b])
                print("I,E,B:{:0>4d},{:0>4d},{:0>4d} weight:{} \nbias:{}".format(
                    j, e, i, rW, rb))

    # predict
    Y_pred = sess.run(reveal_Y, feed_dict={X: real_X, Y: real_Y})
    print("Y_pred:", Y_pred)
```

Please refer to [rtt-linear_regression_reveal.py](../example/tutorials/code/rtt-linear_regression_reveal.py) for the complete source code.

Then, we run it as follows:

```sh
./tutorials.sh rtt linear_regression_reveal
```

And, the output will be as follows:

```log
Y_pred: [[b'4.844925']
 [b'5.297165']
 [b'5.819885']
 ...
 [b'4.992172']
 [b'5.845917']
 [b'6.159866']]
```

Try to compare this output with the output of the `TensorFlow` version to see how much the error is.

#### Comparison and Evaluation 1

Here is the tutorial, you can get both the predicted value and weight value of `TensorFlow` version and `Rosetta` version.

For models with few parameters, (the error in the previous section) can be barely recognized at the first sight, but if there are many parameters and the dataset is very large, then auxiliary tools are needed.

We only list the final comparison results here. For details, refer to `Comparison and Evaluation 2`.

Below is a comparison of the evaluation result of `TensorFlow` and `Rosetta`.

TensorFlow:

```json
{
  "tag": "tensorflow",
  "mse": 0.5228572335042407,
  "rmse": 0.7230886761001314,
  "mae": 0.4290781021000001,
  "evs": 0.2238489236789002,
  "r2": 0.18746385319936198
}
```

Rosetta:

```json
{
  "tag": "rosetta",
  "mse": 0.5219412461669367,
  "rmse": 0.72245501324784,
  "mae": 0.4286960000000004,
  "evs": 0.2244437402223386,
  "r2": 0.18888732556213872
}
```

We can see that the evaluation scores (with little loss on precision) are almost the same.

> R^2 is lower because this dataset is a Logistic Regression model, not a Linear Regression model
> Here we only need to care about the error between the two versions (it is very small)

<details>
  <summary><mark><font color=darkred>Error comparison (linear regression)</font></mark></summary>

The following figure is about the absolute error comparison between the predicted values ​​of `TensorFlow` and `Rosetta`.

![linear_regression_stat-Y-diff](./_static/tutorials/linear_regression_stat-Y-diff.png)


The following figure is about the relative error comparison between the predicted values ​​of `TensorFlow` and `Rosetta`.

![linear_regression_stat-Y-diff4](./_static/tutorials/linear_regression_stat-Y-diff4.png)

</details>


#### Comparison and Evaluation 2

We only show the final result in `Comparison and Evaluation 1` section. Here we dive a little deeper with the inner process. You can skip this section if not interested.

> In this section, Linear Regression is evaluated using R^2, and Logistic Regression is evaluated using AUC/ACC/F1 Evaluation.

<br/>

Next, let’s modify the last part of the program and add the statistical functionality (this modification is the same for the `TensorFlow` version and the `Rosetta` version)

```py
# #############################################################
# save to csv for comparing, for debug
scriptname = os.path.basename(sys.argv[0]).split(".")[0]
csvprefix = "./log/" + scriptname
os.makedirs(csvprefix, exist_ok=True)
csvprefix = csvprefix + "/tf"
# #############################################################

with tf.Session() as sess:
    sess.run(init)
    xW, xb = sess.run([W, b])
    print("init weight:{} \nbias:{}".format(xW, xb))

    # train
    BATCHES = math.ceil(len(real_X) / BATCH_SIZE)
    for e in range(EPOCHES):
        for i in range(BATCHES):
            bX = real_X[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            bY = real_Y[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            sess.run(train, feed_dict={X: bX, Y: bY})

            j = e * BATCHES + i
            if j % 50 == 0 or (j == EPOCHES * BATCHES - 1 and j % 50 != 0):
                xW, xb = sess.run([W, b])
                print("I,E,B:{:0>4d},{:0>4d},{:0>4d} weight:{} \nbias:{}".format(
                    j, e, i, xW, xb))
                savecsv("{}-{:0>4d}-{}.csv".format(csvprefix, j, "W"), xW)
                savecsv("{}-{:0>4d}-{}.csv".format(csvprefix, j, "b"), xb)

    # predict
    Y_pred = sess.run(pred_Y, feed_dict={X: real_X, Y: real_Y})
    print("Y_pred:", Y_pred)
    savecsv("{}-pred-{}.csv".format(csvprefix, "Y"), Y_pred)

    # save real y for evaluation
    savecsv("{}-real-{}.csv".format(csvprefix, "Y"), real_Y)
```

Please refer to [tf-linear_regression_stat.py](../example/tutorials/code/tf-linear_regression_stat.py) for the complete code.

Please refer to  [rtt-linear_regression_stat.py](../example/tutorials/code/rtt-linear_regression_stat.py) for the complete code.

Then, you will get the evaluation results after running it as follows:

```sh
./tutorials.sh tf linear_regression_stat
./tutorials.sh rtt linear_regression_stat
./tutorials.sh stat linear_regression_stat linear
```

#### Model Saving

So far, we just output the model parameters and predicted values ​​to the terminal. How can we save the trained model as we often do in machine learning?

You may wonder, since we are doing all these in a multi-party way, WHERE will the trained model (should) be saved after running with `Rosetta`? And HOW to save it? Good question. Let’s talk about the model saving.

There are several conventions:

- If you want to use `Rosetta` for prediction on shared private dataset, please save the model as `cipher text`.

- If you save the model as plain text, and want to use this model to make predictions on plaintext, please use `TensorFlow` directly to make predictions.

Regarding the saving of the plaintext result, you can choose to save at node 0, node 1, node 2, or all three nodes. This setting is in the configuration file.

> You can try to modify the value of `SAVER_MODE` in the configuration file to see how it works.

`SAVER_MODE` is a flag of a bitmap combination, and its meaning is as follows

```sh
// 0: Save the cipher text. (And the number 1 ~ 7 below indicates which specific parties to  save the plain text files)
// 1: P0,
// 2: P1,
// 4: P2,
// 3: P0 and P1
// 5: P0 and P2
// 6: P1 and P2
// 7: P0, P1 and P2
```

<br/>

In this section, we will use `Rosetta` to train the model, then save the model as `plaintext`, and then load this `plaintext model` into the `TensorFlow` version for prediction. Finally we check the difference between using `TensorFlow` on plaintext dataset without data privacy with this one.

<br/>

Based on the previous version of `Rosetta`, we added some code related to `save`.

Before training starts:

```py
# save
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')
os.makedirs("./log/ckpt"+str(party_id), exist_ok=True)
```

After training:

```py
saver.save(sess, './log/ckpt'+str(party_id)+'/model')
```

Please refer to [rtt-linear_regression_saver.py](../example/tutorials/code/rtt-linear_regression_saver.py) for details.

And then run it as follows:

```sh
./tutorials.sh rtt linear_regression_saver
```

#### Model Loading and Prediction

The model has been saved to the corresponding node in the previous step (according to the configuration file). Now use `TensorFlow` to load the plaintext model saved in the previous step and make predictions.

```py
# save
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')
os.makedirs("./log/ckpt0", exist_ok=True)

# restore mpc's model and predict
with tf.Session() as sess:
    sess.run(init)
    if os.path.exists("./log/ckpt0/checkpoint"):
        saver.restore(sess, './log/ckpt0/model')

    # predict
    Y_pred = sess.run(pred_Y)
    print("Y_pred:", Y_pred)
```

For the complete code, please refer to [tf-linear_regression_restore.py](../example/tutorials/code/tf-linear_regression_restore.py).

And then run it as follows:

```sh
./tutorials.sh tf linear_regression_restore
```

The output will be as follows:

```log
Y_pred: [[6.17608922]
 [6.15961048]
 [5.40468624]
 ...
 [5.20862467]
 [5.49407074]
 [6.21659464]]
```

<br/>

Summary:

Complete source code list reference

TensorFlow version

|                              |                                                                                              |
| ---------------------------- | -------------------------------------------------------------------------------------------- |
| Basics                       | [tf-linear_regression.py](../example/tutorials/code/tf-linear_regression.py)                 |
| Model Training and Saving    | [tf-linear_regression_saver.py](../example/tutorials/code/tf-linear_regression_saver.py)     |
| Model loading and prediction | [tf-linear_regression_restore.py](../example/tutorials/code/tf-linear_regression_restore.py) |
| Evaluation                   | [tf-linear_regression_stat.py](../example/tutorials/code/tf-linear_regression_stat.py)       |

Rosetta version

|                                       |                                                                                                |
| ------------------------------------- | ---------------------------------------------------------------------------------------------- |
| Basics                                | [rtt-linear_regression.py](../example/tutorials/code/rtt-linear_regression.py)                 |
| Basic (output plain text)             | [rtt-linear_regression_reveal.py](../example/tutorials/code/rtt-linear_regression_reveal.py)   |
| Model Training and Saving             | [rtt-linear_regression_saver.py](../example/tutorials/code/rtt-linear_regression_saver.py)     |
| Model (Cipher) loading and prediction | [rtt-linear_regression_restore.py](../example/tutorials/code/rtt-linear_regression_restore.py) |
| Evaluation                            | [rtt-linear_regression_stat.py](../example/tutorials/code/rtt-linear_regression_stat.py)       |

### Logistic Regression

With the foundation of `Linear Regression` above, then `Logistic Regression` is `very`, `very`, `very simple`.

Based on Linear Regression, we use `sigmoid` as a binary classifier and `cross entropy` as a loss function to build a Logistic Regression model.

Regardless of the `TensorFlow` version or the`Rosetta` version, the changes are the same. Compared with the `Linear Regression` version, only the model construction part needs to be changed, that is, only the following modification is needed:

- On the predicted value part, add `sigmoid` functionality

- On the loss function part, we replace them with cross entropy function

```py
# predict
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)

# loss
logits = tf.matmul(X, W) + b
loss = tf.nn.sigmoid_cross_entropy_with_logits(labels=Y, logits=logits)
loss = tf.reduce_mean(loss)
```

Complete source code list reference:

TensorFlow version

|                              |                                                                                                  |
| ---------------------------- | ------------------------------------------------------------------------------------------------ |
| Basics                       | [tf-logistic_regression.py](../example/tutorials/code/tf-logistic_regression.py)                 |
| Model Training and Saving    | [tf-logistic_regression_saver.py](../example/tutorials/code/tf-logistic_regression_saver.py)     |
| Model loading and prediction | [tf-logistic_regression_restore.py](../example/tutorials/code/tf-logistic_regression_restore.py) |
| Evaluation                   | [tf-logistic_regression_stat.py](../example/tutorials/code/tf-logistic_regression_stat.py)       |

Rosetta version

|                                       |                                                                                                    |
| ------------------------------------- | -------------------------------------------------------------------------------------------------- |
| Basics                                | [rtt-logistic_regression.py](../example/tutorials/code/rtt-logistic_regression.py)                 |
| Basic (output plain text)             | [rtt-logistic_regression_reveal.py](../example/tutorials/code/rtt-logistic_regression_reveal.py)   |
| Model Training and Saving             | [rtt-logistic_regression_saver.py](../example/tutorials/code/rtt-logistic_regression_saver.py)     |
| Model (Cipher) loading and prediction | [rtt-logistic_regression_restore.py](../example/tutorials/code/rtt-logistic_regression_restore.py) |
| Evaluation                            | [rtt-logistic_regression_stat.py](../example/tutorials/code/rtt-logistic_regression_stat.py)       |

Just run these programs in the same as in `Linear Regression`.

<br/>

Here is the comparison of the two different versions of Logistic Regression model.

TensorFlow:

```json
{
  "tag": "tensorflow",
  "score_auc": 0.698190821826193,
  "score_ks": 0.2857188520398128,
  "threshold_opt": 0.6037812829,
  "score_accuracy": 0.6458170445660673,
  "score_precision": 0.6661931818181818,
  "score_recall": 0.6826783114992722,
  "score_f1": 0.6743350107836089
}
```

Rosetta:

```json
{
  "tag": "rosetta",
  "score_auc": 0.6977740568078996,
  "score_ks": 0.2857188520398128,
  "threshold_opt": 0.607208,
  "score_accuracy": 0.6458170445660673,
  "score_precision": 0.6661931818181818,
  "score_recall": 0.6826783114992722,
  "score_f1": 0.6743350107836089
}
```

> Here we only care about the error between the two versions (we can see that it is very small)
> Rosetta is even slightly better than TensorFlow

<details>
  <summary><mark><font color=darkred>Error comparison (logistic regression)</font></mark></summary>

The following figure is about the absolute error comparison between the predicted values ​​of `TensorFlow` and `Rosetta`.

![logistic_regression_stat-Y-diff](./_static/tutorials/logistic_regression_stat-Y-diff.png)


The following figure is about the relative error comparison between the predicted values ​​of `TensorFlow` and `Rosetta`.

![logistic_regression_stat-Y-diff4](./_static/tutorials/logistic_regression_stat-Y-diff4.png)


**Warm tip:** There may be individual `rtt predictions value` here that differ significantly from `tf predictions value` (but do not affect scoring) for the following reason:
- The Rosetta uses a 6-segment (interval [-4,4]) function to simulate sigmoid that sets the output value to 0 or 1 when the sigmoid input is less than -4 or greater than 4. This results in a larger when computing (`tf prediction value` - `rtt prediction value`)/`rtt prediction value`.
- As shown above, for the 425th, 727th sample, the input values of sigmoid are -4.1840362549 and -4.6936187744, respectively.


</details>


### Support big data sets

The above linear regression and logistic regression models all load the entire dataset into memory and then take it out in batch order for training, and as the size of the dataset grows, it becomes impractical to load the dataset into memory at once.

Major plaintext AI frameworks such as TensorFlow are aware of and provide solutions, TensorFlow provides the relevant Dataset APIs to build low-memory consuming, complex, reusable data pipelines, since Rosetta uses TensorFlow as a backend, it can be reused with minor modifications.

We use logistic regression model as an example to illustrate how to train a model with large datasets.

For the TensorFlow version complete code, please refer to [tf-ds-lr.py](../example/tutorials/code/tf-ds-lr.py).

For the Rosetta version complete code, please refer to [rtt-ds-lr.py](../example/tutorials/code/rtt-ds-lr.py).

Analysis of the code in tf-ds-lr.py and rtt-ds-lr.py reveals two main differences.

1. Create a text line dataset, use TextLineDataset class in TensorFlow and use PrivateTextLineDataset class in Rosetta.
    The code used in TensorFlow is as following:

    ```py
    dataset_x = tf.data.TextLineDataset(file_x)
    ...
    ```

    The code used in Rosetta is as following:

    ```py
    dataset_x0 = rtt.PrivateTextLineDataset(
                    file_x, data_owner=0)  # P0 hold the file data
    ...
    ```

2. Decode functions are implemented differently. TensorFlow version of the decode function split rows to corresponding fields and then converts the fields to floating-point numbers, while the Rosetta version of the decode function also first split rows to corresponding fields and then calls `PrivateInput` function to share the data.
    The code used in TensorFlow is as following:

    ```py
    # dataset decode
    def decode_x(line):
        fields = tf.string_split([line], ',').values
        fields = tf.string_to_number(fields, tf.float64)
        return fields
    ```

    The code used in Rosetta is as following:

    ```py
    # dataset decode
    def decode_p0(line):
        fields = tf.string_split([line], ',').values
        fields = rtt.PrivateInput(fields, data_owner=0) # P0 hold the file data
        return fields
    ```
### Support stop Rosetta execution at any time
The `Rosetta` backend uses the `TensorFlow` execution engine to execute the computational graph constructed by Rosetta's privacy operations, so you can use the same method as `TensorFlow` to stop the graph at any time during the training of a privacy AI model. To stop the execution of the graph in `TensorFlow`, we simply call python API `Session::close()`, so in `Rosetta` we can also stop the execution of the computed graph by calling the python API `Session::close()`.

## Privacy-Preserving Deep Learning

### MLP Neural Network

After reading the content of [Privacy-Preserving Machine Learning](#Privacy-Preserving Machine Learning), you must  have a certain understanding of Rosetta and TensorFlow 's grammar. So, in this section, we have offer an example of classification of mnist dataset by MLP Neural Network.

Firstly, this is the TensorFlow version 

#### TensorFlow Version MLP

- Import related packages and load dataset

```python
from tensorflow.examples.tutorials.mnist import input_data
import os
import tensorflow as tf
mnist_home = os.path.join("/tmp/data/", 'mnist')
mnist = input_data.read_data_sets(mnist_home, one_hot=True)
# split the data into train and test
X_train = mnist.train.images
X_test = mnist.test.images
Y_train = mnist.train.labels
Y_test = mnist.test.labels
# make iterator
train_dataset = tf.data.Dataset.from_tensor_slices((X_train, Y_train))
train_dataset = train_dataset.batch(100).repeat()
test_dataset = tf.data.Dataset.from_tensor_slices((X_test, Y_test))
test_dataset = test_dataset.batch(100).repeat()
train_iterator = train_dataset.make_one_shot_iterator()
train_next_iterator = train_iterator.get_next()
test_iterator = test_dataset.make_one_shot_iterator()
test_next_iterator = test_iterator.get_next()
```

- Set hyperparameters and construct MLP model

```python
num_outputs = 10 
num_inputs = 784
w=[]
b=[]

def mlp(x, num_inputs, num_outputs, num_layers, num_neurons):
    w = []
    b = []
    for i in range(num_layers):
        # weights
        w.append(tf.Variable(tf.random_normal(
            [num_inputs if i == 0 else num_neurons[i - 1],
             num_neurons[i]], seed = 1, dtype=tf.float64),
            name="w_{0:04d}".format(i), dtype=tf.float64
        ))
        # biases
        b.append(tf.Variable(tf.random_normal(
            [num_neurons[i]], seed = 1, dtype=tf.float64),
            name="b_{0:04d}".format(i), dtype=tf.float64
        ))
    w.append(tf.Variable(tf.random_normal(
        [num_neurons[num_layers - 1] if num_layers > 0 else num_inputs,
         num_outputs], seed = 1, dtype=tf.float64), name="w_out", dtype=tf.float64))
    b.append(tf.Variable(tf.random_normal([num_outputs], seed = 1, dtype=tf.float64), name="b_out", dtype=tf.float64))

    # x is input layer
    layer = x
    # add hidden layers
    for i in range(num_layers):
        layer = tf.nn.relu(tf.matmul(layer, w[i]) + b[i])
    # add output layer
    layer = tf.matmul(layer, w[num_layers]) + b[num_layers]

    return layer
```

- Implement training function and related functions

```python
def mnist_batch_func(batch_size=100):
    X_batch, Y_batch = mnist.train.next_batch(batch_size)
    return [X_batch, Y_batch]
  
def tensorflow_classification(n_epochs, n_batches,
                              batch_size,
                              model, optimizer, loss, accuracy_function,
                              X_test, Y_test):
    with tf.Session() as tfs:
        tfs.run(tf.global_variables_initializer())
        for epoch in range(n_epochs):
            epoch_loss = 0.0
            for batch in range(n_batches):
                X_batch, Y_batch = tfs.run(train_next_iterator)
                feed_dict = {x: X_batch, y: Y_batch}
                _, batch_loss = tfs.run([optimizer, loss], feed_dict)
                epoch_loss += batch_loss
        
            average_loss = epoch_loss / n_batches
            print("epoch: {0:04d} loss = {1:0.6f}".format(
                epoch, average_loss))
        feed_dict = {x: X_test, y: Y_test}
        accuracy_score = tfs.run(accuracy_function, feed_dict=feed_dict)
        print("accuracy={0:.8f}".format(accuracy_score))
        
# construct input
x = tf.placeholder(dtype=tf.float64, name="x", 
                    shape=[None, num_inputs])
# construct output
y = tf.placeholder(dtype=tf.float64, name="y", 
                    shape=[None, 10])
# hidden layers' parameters
num_layers = 2
num_neurons = [128, 256]
learning_rate = 0.01
n_epochs = 30
batch_size = 100
n_batches = int(mnist.train.num_examples/batch_size)

model = mlp(x=x,
            num_inputs=num_inputs,
            num_outputs=num_outputs,
            num_layers=num_layers,
            num_neurons=num_neurons)

loss = tf.reduce_mean(
    tf.nn.sigmoid_cross_entropy_with_logits(logits=model, labels=y))
optimizer = tf.train.GradientDescentOptimizer(
    learning_rate=learning_rate).minimize(loss)

predictions_check = tf.equal(tf.argmax(model, 1), tf.argmax(y, 1))
accuracy_function = tf.reduce_mean(tf.cast(predictions_check, dtype=tf.float64))
# train
tensorflow_classification(n_epochs=n_epochs, 
   n_batches=n_batches, 
   batch_size=batch_size, 
   model = model, 
   optimizer = optimizer, 
   loss = loss, 
   accuracy_function = accuracy_function, 
   X_test = X_test, 
   Y_test = Y_test
   )
```

For the complete code, please refer to [tf-mlp_mnist.py](../example/tutorials/code/tf-mlp_mnist.py)

Run it as follows:

```python
python ./tf-mlp_mnist.py
```

Output as follows:

```python
epoch: 0000 loss = 17.504000
epoch: 0001 loss = 6.774922
epoch: 0002 loss = 4.993065
epoch: 0003 loss = 4.047511
epoch: 0004 loss = 3.440471
epoch: 0005 loss = 3.006049
  ...
epoch: 0027 loss = 0.874316
epoch: 0028 loss = 0.847307
epoch: 0029 loss = 0.821776
accuracy=0.91100000
```

#### Rosetta Version MLP

- Import packages and activate protocol

```python
import os
import tensorflow as tf
import latticex.rosetta as rtt
import csv
import numpy as np

rtt.set_backend_loglevel(1)
np.set_printoptions(suppress=True)
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
np.random.seed(0)
rtt.activate("SecureNN")
mpc_player_id = rtt.py_protocol_handler.get_party_id()
```

- Load dataset

If you want to understand the method of loading dataset, please refer to [Support big data sets](#Support big data sets).

```python
# load data
file_x = '../dsets/P' + str(mpc_player_id) + "/mnist_train_x.csv"
file_y = '../dsets/P' + str(mpc_player_id) + "/mnist_train_y.csv"
X_train_0 = rtt.PrivateTextLineDataset(file_x, data_owner=0)
X_train_1 = rtt.PrivateTextLineDataset(file_x, data_owner=1)
Y_train = rtt.PrivateTextLineDataset(file_y, data_owner=1)

# dataset decode
def decode_p0(line):
    fields = tf.string_split([line], ',').values
    fields = rtt.PrivateInput(fields, data_owner=0)
    return fields
def decode_p1(line):
    fields = tf.string_split([line], ',').values
    fields = rtt.PrivateInput(fields, data_owner=1)
    return fields
  
# dataset pipeline
X_train_0 = X_train_0.map(decode_p0).cache(f"{cache_dir}/cache_p0_x0").batch(BATCH_SIZE).repeat()
X_train_1 = X_train_1.map(decode_p1).cache(f"{cache_dir}/cache_p1_x1").batch(BATCH_SIZE).repeat()
Y_train = Y_train.map(decode_p1).cache(f"{cache_dir}/cache_p1_y").batch(BATCH_SIZE).repeat()
```

- The model can be saved in the folder `log/`

```python
def tensorflow_classification(n_epochs, n_batches,
                              batch_size,
                              model, optimizer, loss
                              ):
    with tf.Session() as tfs:
        tfs.run(tf.global_variables_initializer())
        tfs.run([iter_x0.initializer, iter_x1.initializer, iter_y.initializer])
        for epoch in range(n_epochs):
            epoch_loss = 0.0
            for i in range(n_batches):
                tfs.run([optimizer, loss])
        saver.save(tfs, './log/ckpt'+str(mpc_player_id)+'/model')
```

The rest is the same as TensorFlow Version MLP, for the complete code, please refer to [rtt-mlp_mnist.py](../example/tutorials/code/rtt-mlp_mnist.py)

Run it as follows:

```python
./tutorials.sh rtt mlp_mnist
```

Plaintext model can be saved at folder `log/`, you can the follow code to evaluate the mode

```python
python tf-test_mlp_acc.py
```

The example of results as follows:

```python
['w_out:0', 'b_out:0']
[array([[ 0.24279785, -0.35299683, -0.83648682, ..., -1.71618652,
         0.65466309, -0.75320435],
       [ 0.16467285, -1.01171875,  0.07672119, ...,  2.43850708,
        -0.34365845,  0.50970459],
       [ 0.68637085,  0.59738159,  0.21426392, ..., -2.1026001 ,
        -1.08334351, -0.51135254],
       ...,
       [ 1.18951416,  0.50506592, -0.19161987, ...,  0.31906128,
        -0.21728516, -1.74258423],
       [ 0.47128296, -1.10772705, -1.14147949, ..., -0.80792236,
        -0.2272644 , -0.60620117],
       [-1.35250854, -0.00039673, -1.37692261, ...,  0.28158569,
        -1.86367798,  0.2359314 ]]), array([ 0.05230713, -0.48815918, -0.75996399, -0.41955566,  1.78201294,
       -0.42456055, -0.03417969, -1.80670166,  0.40750122, -0.93180847])]
accuracy=0.14000000
```

Due to the model did not add hidden layers and used the mini-datasets, the accuracy is very low. If you are interested in the model, you can adjust the structure of the model and size of the datasets to imporove the model accuracy.

## Support multitasking concurrency

Multitasking is not supported in versions prior to `Rosetta 1.0.0`, which means that only one privacy protocol (e.g. `SecureNN`, `Helix`, etc.) can be executed at any time, and if you want to execute multiple tasks concurrently, you must use multi-process to implement. We have refactored the code in `Rosetta v1.0.0` to support multitasking, allowing different tasks to be executed concurrently using different privacy protocols, meaning that if users have multiple task concurrency requirements, besides multi process implementation, the user business code can also be implemented in multi-threaded way, which provides users with more choices.

Suppose we have a requirement to test the accuracy performance of the `TrueDiv` operation under the `SecureNN` and `Helix` protocols, we can write a case, run it using the `SecureNN` protocol, then modify it to run again using the `Helix` protocol and compare the results. With multitasking support, we can write our cases in a simple but elegant way (here is just a simple test requirement, which can be extended to specific requirements in more specific business scenarios).

This example program with simple requirements is written using multitasking concurrency. The code is as follows:

``` python
import concurrent.futures
import numpy as np
import tensorflow as tf
import latticex.rosetta as rtt


def multi_task_fw(funcs):
    task_id = 1
    all_task = []

    try:
        with concurrent.futures.ThreadPoolExecutor() as executor: 
            for unit_func in funcs:
                all_task.append(executor.submit(unit_func, str(task_id)))
                task_id += 1
    
        concurrent.futures.wait(all_task, return_when=concurrent.futures.ALL_COMPLETED)
    except Exception as e:
        print(str(e))


def run_trurediv_op(protocol, task_id, x_init, y_init):
    local_g = tf.Graph()
    with local_g.as_default():
        X = tf.Variable(x_init)
        Y = tf.Variable(y_init)
        Z = tf.truediv(X, Y)
        rv_Z = rtt.SecureReveal(Z)
        init = tf.compat.v1.global_variables_initializer()

        try:
            rtt.activate(protocol, task_id=task_id)    # Add the task_id parameter
            with tf.Session(task_id=task_id) as sess:  # Add the task_id parameter 
                sess.run(init)        
                real_Z = sess.run(rv_Z)
                print("The result of the truediv calculation using {0} is: {1}".format(protocol, real_Z))
            rtt.deactivate(task_id=task_id)
        except Exception as e:
            print(str(e))


def Snn_Div(task_id):
    return run_trurediv_op("SecureNN", task_id, 
                        [1.1, 1200.5, -1.1, -23489.56], 
                        [102.2, 812435.6, 0.95, 0.1234])


def Helix_Div(task_id):
    return run_trurediv_op("Helix", task_id, 
                        [1.1, 1200.5, -1.1, -23489.56], 
                        [102.2, 812435.6, 0.95, 0.1234])


# run cases
multi_task_fw([Snn_Div, Helix_Div])
```

The output of the above code is as follows: (only the calculation result log is kept, other log is removed)

``` python
The result of the truediv calculation using Helix is:    [b'0.010742' b'0.001465' b'-1.157837' b'-190521.267212']
The result of the truediv calculation using SecureNN is: [b'0.010742' b'0.001465' b'-1.157715' b'-190521.267212']
```

The above code for multitasking support differs from the code before `Rosetta v1.0.0` in only two places.

1. The `rtt.activate` interface adds a `task_id` parameter to bind a specific privacy protocol to a task in order to support multitasking. The `task_id` parameter is optional and if not set will only support single tasks, maintaining compatibility with previous versions.
   
2. The `SecureSession` constructor adds a `task_id` parameter to bind the specific session to a task. so that when the session execution graph is used later the operation layer can find and execute a specific privacy protocol based on `task_id`. The `task_id` parameter is optional, if not set it only supports single tasks, maintaining compatibility with previous versions.
   > Note: The above code uses `tf.Session` and not `rtt.SecureSession` because `tf.Session` is statically overridden by `rtt.SecureSession` and `rtt.SecureSession` is derived from `tf.Session` with the extension parameter `task_id`. (see the implementation of `rtt.SecureSession` for more.)

## Conclusion

That's all.

Now, you have fully mastered the usage of `Rosetta`, go to find a real problem to play with.

welcome!

## Additional Notes

### Dataset Description

The data set source reference [here](http://archive.ics.uci.edu/ml/datasets/Wine+Quality).

We have performed simple pre-process steps to get the results as follows, the path is in `dsets/`, and the directory structure is as follows:

```sh
dsets/
├── ALL
│   ├── cls_test_x.csv
│   ├── cls_test_y.csv
│   ├── cls_train_x.csv
│   ├── cls_train_y.csv
│   ├── reg_test_x.csv
│   ├── reg_test_y.csv
│   ├── reg_train_x.csv
│   ├── reg_train_y.csv
│   ├── mnist_test_x.csv
│   └── mnist_test_y.csv
├── P0
│   ├── cls_test_x.csv
│   ├── cls_test_y.csv
│   ├── cls_train_x.csv
│   ├── cls_train_y.csv
│   ├── reg_test_x.csv
│   ├── reg_train_x.csv
│   └── mnist_train_x.csv
├── P1
│   ├── cls_test_x.csv
│   ├── cls_train_x.csv
│   ├── reg_test_x.csv
│   ├── reg_test_y.csv
│   ├── reg_train_x.csv
│   ├── reg_train_y.csv
│   └── mnist_train_x.csv
└── P2
```

|        |                                                                     |
| ------ | ------------------------------------------------------------------- |
| ALL    | Raw data of the dataset                                             |
| P*     | Indicates the private data owned by each node                       |
| cls*   | Represents a binary classification data set for Logistic Regression |
| reg*   | Regression data set, used for Linear Regression                     |
| *train | Represents the data set used for training                           |
| *test  | Represents the data set used for prediction                         |
| *x     | Indicates sample                                                    |
| *y     | Means label                                                         |

Description:

For comparison with the plaintext (TensorFlow version), we divided the original data set into two parts in the vertical direction, one as private data of `P0` and the other as private data of`P1`.

1. The data under ALL is used for TensorFlow version.
2. The private data of each node of P0/P1 is stored on each node.
3. P2 has no data.
4. The label for Logistic Regression is owned by P0, and the label for Linear Regression is owned by P1.
