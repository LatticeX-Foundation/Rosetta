- [概述](#概述)
- [安装部署](#安装部署)
- [快速入门](#快速入门)
- [安全多方计算](#安全多方计算)
  - [百万富翁](#百万富翁)
    - [tensorflow 版本](#tensorflow-版本)
    - [rosetta 版本](#rosetta-版本)
- [隐私机器学习](#隐私机器学习)
  - [线性回归](#线性回归)
    - [tensorflow 版本](#tensorflow-版本-1)
    - [rosetta 基础版](#rosetta-基础版)
    - [rosetta 版本-Reveal](#rosetta-版本-reveal)
    - [对比与评估 1](#对比与评估-1)
    - [对比与评估 2](#对比与评估-2)
    - [模型保存](#模型保存)
    - [模型加载与预测](#模型加载与预测)
  - [逻辑回归](#逻辑回归)
  - [支持超大数据集](#支持超大数据集)
- [结语](#结语)
- [附加](#附加)
  - [数据集说明](#数据集说明)


## 概述

## 安装部署

如果你还没有搭建 `rosetta` 环境，请参考[部署文档](./DEPLOYMENT_CN.md)。

为了方便展开教程，这里的示例是 `单机多节点` 的，请你参考 `部署文档` 以此种方式进行部署。

## 快速入门

下面所有的内容，都是基于你已经完成了前面的安装部署。为了节省时间，请确保环境已经 `OK`。

若无特别说明，所有命令行的运行路径都在 `example/tutorials/code/`。

<br/>

接下来，让我们一起进入激动时刻，如何以`最快的速度`使用 `rosetta` 呢？

方法很简单，在任何你需要使用 `rosetta` 的地方（`Python` 脚本文件），导入我们的 `rosetta` 包即可，如下：

```python
import latticex.rosetta as rtt
```

<font style="color:green">注：</font> `rtt` 对应于 `rosetta`，就如同 `tf` 之 `tensorflow`，`np` 之 `numpy`，`pd` 之 `pandas`，是一个约定。

<br/>

你可以在同一个终端下，直接运行

```sh
./tutorials.sh rtt quickstart
```


或者，也可以在三个不同的终端（你可以理解为，这三个不同的终端，模拟了三个不同主机节点）下，分别运行如下：


```sh
# node 0
python3 quickstart.py --party_id=0
```

```sh
# node 1
python3 quickstart.py --party_id=1
```

```sh
# node 2
python3 quickstart.py --party_id=2
```

如果输出了 `DONE!`，那么表示 OK 了。

> `--party_id` 这个是命令行选项，指定了当前脚本是执行的哪一方。

> 为了节省文本，后文直接使用 `./tutorials.sh` 快速运行，不再以显示使用三个终端的形式运行。

<br/>

接下来的教程，就如同这个 `快速入门` 一样的轻松。


## 安全多方计算


话说，有 `两个` `诚实的` 有钱人在讨论一件非常有趣的事情 — `谁的财富更多`，但是呢，谁都不愿意说出自己具体拥有多少财富。那怎么办，我是没有办法的，但 `Rosetta` 可以帮你。下面让我们一起来看看这个问题如何解决。


### 百万富翁

百万富翁的问题，其描述可以参考[这里]。

假设两个富翁一个叫 `Alice`，一个叫 `Bob`，分别拥有 `2000001` 和 `2000000` 美元。你没有看错，两个人的财富只差 `1` 美元。


#### tensorflow 版本

我们先来看一下这种（假设的）情况：两个富翁把各自拥有多财富说出来。

这种情况非常简单，心算即可以解决，但为了与 `rosetta` 进行对比，这里用 `tensorflow` 写一个比较程序。步骤如下：


第一步，导入包。

```py
import tensorflow as tf
```

第二步，设置各有多少财富。

```py
Alice = tf.Variable(2000001)
Bob = tf.Variable(2000000)
```

第三步，调用 session.run，并接收结果。

```py
res = tf.greater(Alice, Bob)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    ret = sess.run(res)
    print("ret:", ret)  # ret: True
```

完整代码参考 [tf-millionaire.py](../example/tutorials/code/tf-millionaire.py)。

执行 

```sh
./tutorials.sh tf millionaire
```

输出如下：

```log
ret: True
```

结果表示 `Alice` 的财富多于 `Bob` 的财富。


很简单，不赘述。

#### rosetta 版本

上面是一种假设的情况，那么在真实情况下，如何使用 `rosetta` 解决百万富翁的的问题呢，让我们一起来看看吧。非常简单！

第一步，导入 `rosetta` 包。

```py
import latticex.rosetta as rtt
import tensorflow as tf
```


第二步，激活协议。此处选择 `SecureNN` 协议。

```py
rtt.rtt.activate("SecureNN")
```


第三步，设置各有多少财富。

我们通过内置的 `rtt.private_console_input` 来获取用户在终端输入的私有数据。

```py
Alice = tf.Variable(rtt.private_console_input(0))
Bob = tf.Variable(rtt.private_console_input(1))
```

第四步，与 `tensorflow` 完全一样。

```py
res = tf.greater(Alice, Bob)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    res = sess.run(res)
    print('res:', res)  # res: 722739251331272.4
```

上面的输出的 `res` 是一个 `sharing` 值。


> `sharing` 值，即 `秘密分享` 值，一个原始值 x，随机拆分成两个 64bit 的值 x0, x1 （x = x0 + x1），分别由 P0 与 P1 执有。


那如何知道明文值呢，我们提供了一个 `reveal` 接口，用来获取明文值。加在第四步的后面即可：

```py
with tf.Session() as sess:
    # ...
    ret = rtt.SecureReveal(res)
    print('ret:', sess.run(ret))  # ret: 1.0
```

控制台版本，完整代码参考 [rtt-millionaire-console.py](../example/tutorials/code/rtt-millionaire-console.py)。

脚本版本，完整代码参考 [rtt-millionaire.py](../example/tutorials/code/rtt-millionaire.py)。

执行 

```sh
./tutorials.sh rtt millionaire
```

输出如下：

```log
ret: 1.0
```

结果表示 `Alice` 的财富多于 `Bob` 的财富。

<br/>

> 包括 `SecureReveal` 在内的所有 `rosetta` 支持的算子的说明，参考[AIP 文档](./API_DOC_CN.md)。

<br/>

当然，`rosetta` 不仅仅只能用来解决如 `百万富翁` 这样简单的应用，下面我们来看看 `rosetta` 与 `机器学习（Meachine Learning）` 的结合。

## 隐私机器学习

这部分讲讲，`隐私`与`机器学习（Meachine Learning，ML）`的结合。先从最简单的机器学习之`线性回归（Linear Regression）`开始。

### 线性回归

本节从 `数据处理`/`训练与模型保存`/`模型加载与预测`/`评估` 这几个方面介绍如何使用 `rosetta` 进行`线性回归`。

在使用 `rosetta` 进行机器学习之前，为了与 `rosetta` 版本对比，这里先引入一个 `完全对等` 的 `tensorflow` 版本。

我们首先来看看这个 `tensorflow` 版本。

#### tensorflow 版本

这里是一个简单的线性回归。

- 导入必要的包，设定训练参数等。

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

EPOCHES = 100
BATCH_SIZE = 16
learning_rate = 0.0002
```

- 加载数据集

数据集说明参考本文最后的附录。

我们标识了不同于 `rosetta` 的地方，后文可更多地关注一些。

```py
# real data
# ######################################## difference from rosettta
file_x = '../dsets/ALL/reg_train_x.csv'
file_y = '../dsets/ALL/reg_train_y.csv'
real_X, real_Y = pd.read_csv(file_x).to_numpy(), pd.read_csv(file_y).to_numpy()
# ######################################## difference from rosettta
DIM_NUM = real_X.shape[1]
```

- 编写线性回归模型。

此部分不赘述，只列出代码，详情参考 `TensorFlow™` 官网关于机器学习模型的编写。

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


- 模型训练。

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

完整代码参考 [tf-linear_regression.py](../example/tutorials/code/tf-linear_regression.py)。


执行 

```sh
./tutorials.sh tf linear_regression
```

输出如下：

```log
Y_pred: [[5.409453 ]
 [5.803287 ]
 [5.9634194]
 ...
 [4.978249 ]
 [5.9761114]
 [5.9929996]]
```



#### rosetta 基础版


如前文所述，如果你已经有了一个用 `tensorflow` 编写模型训练脚本(.py)，那么你要做的事情就是，只是在这个脚本文件的第一行，导入如下包即可：

```python
import latticex.rosetta as rtt
```

**是的，就是这么简单！** 你不需要修改任何已经写好的代码。即使你对 密码学 没有任何的知识，也可以使用。


- 激活协议

```py
rtt.rtt.activate("SecureNN")
```

> 注：在使用任何 `MPC` 相关 `API` 之前必须先激活协议。


- 加载数据集

数据集说明参考本文最后的附录。


我们标识了不同于 `tensorflow` 的地方，对照着 `tensorflow` 版本，除了导入了 `rosetta` 包外，就只有这一处是不同的。

`rosetta` 提供了一个专门于处理私有数据集的类，`PrivateDataset`。详情查阅相关源代码。

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

完整代码参考 [rtt-linear_regression.py](../example/tutorials/code/rtt-linear_regression.py)。

<br/>

OK，简单总结一下与 `tensorflow` 版本的区别：

- 导入 `rosetta` 包。
- 激活协议。
- 数据集的加载。

<br/>

到现在，我们已经完成了编码，怎么运行呢？

还记得前面的 `百万富翁` 的问题吗，这里运行的方法是一样的。

执行 

```sh
./tutorials.sh rtt linear_regression
```

输出如下：

```log
Y_pred: [[1.22580022e+14]
 [1.22481157e+14]
 [1.22514398e+14]
 ...
 [1.22532401e+14]
 [1.22508954e+14]
 [1.22495981e+14]]
```

没错，你看到是 sharing 值。


#### rosetta 版本-Reveal

上一节输出的 `sharing` 值，根本无法阅读！为了测试/调试，或与明文比对，或其他高级操作，我们提供了一个 `reveal` 接口，用要获取明文值。

<font style="color:red">温馨提示：不建议或谨慎在生产环境中使用这个 reveal 接口。</font>

我们稍微修改一下前面的（基础版）程序，加上 `reveal`，再看看有什么效果，如下：

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

完整代码参考 [rtt-linear_regression_reveal.py](../example/tutorials/code/rtt-linear_regression_reveal.py)。


执行 

```sh
./tutorials.sh rtt linear_regression_reveal
```

输出如下：

```log
Y_pred: [[5.40625 ]
 [5.828125]
 [5.953125]
 ...
 [5.      ]
 [5.984375]
 [5.984375]]
```


尝试着将这个输出与 `tensorflow` 版本的输出比对一下，看看误差有多少。



#### 对比与评估 1

教程进行到这里，已经可以得到 `tensorflow` 版本与 `rosetta` 版本的预测值与权重值。

对于参数较少的模型，（上一节的误差）肉眼还可以勉强识别，可如果参数非常多，数据集非常大，这时就需要辅助工具了。

这里只列出对比结果，详情参考 `对比与评估 2`。

下面是 `tensorflow` 与 `rosetta` 的评分对比。

tensorflow:

```json
{
  "tag": "tensorflow",
  "mse": 0.5182142087177698,
  "rmse": 0.7198709667140145,
  "mae": 0.4328875541499997,
  "evs": 0.22396289848005935,
  "r2": 0.19491626081852909
}
```

rosetta:

```json
{
  "tag": "rosetta",
  "mse": 0.5210866435592723,
  "rmse": 0.7218633136261132,
  "mae": 0.421875,
  "evs": 0.2204962547250663,
  "r2": 0.19045372284900097
}
```

我们看到了，`tensorflow` 与 `rosetta` 对比，其评分（误差对比）结果还是不错的。


> R^2 比较低是因为这个数据集是逻辑回归模型，不是线性回归模型

> 此处只需要关心两个版本之间的误差（是非常小的）


下图是关于 `tensorflow` 与 `rosetta` 预测值的误差对比。

![](./_static/tutorials/linear_regression_stat-Y-diff4.png)


#### 对比与评估 2

`对比与评估 1` 只列出了结果，这里补充说明一下，这里属于高级部分。可以跳过此节，不影响阅读。


> 本节中，线性回归使用 R^2 进行评估，逻辑回归使用 AUC/ACC/F1 进行评估。

<br/>

下面，我们修改一下程序的最后一部分，加入统计代码（此修改对 `tesnorflow` 版本与 `rosetta` 版本是一样的）

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

完整代码参考 [tf-linear_regression_stat.py](../example/tutorials/code/tf-linear_regression_stat.py)。

完整代码参考 [rtt-linear_regression_stat.py](../example/tutorials/code/rtt-linear_regression_stat.py)。


执行 

```sh
./tutorials.sh tf linear_regression_stat
./tutorials.sh rtt linear_regression_stat
./tutorials.sh stat linear_regression_stat linear
```


#### 模型保存

至此，我们只是将模型参数或预测值输出到了终端，那怎样保存训练好的模型呢？

有人会问，我们是多方训练，`rosetta` 训练后得到的模型（应该）保存在哪里呢？怎样保存的？问得好，下面就来讲讲，模型的事儿。


有几个约定：

- 如果要用 `rosetta` 进行预测，请将模型保存为密文。

- 如果将模型保存为明文，但又要用此模型进行预测，请直接使用 `tensorflow` 进行预测。

关于明文的保存，你可以选择保存在 节点0，节点1，节点2，或者三个节点都各自保存一份。这个设置在配置文件中。

> 你可以尝试修改配置文件中 `SAVER_MODE` 的值，看看效果如何。

`SAVER_MODE` 是一按位组合的标志，其含义如下

```
//  0: 保存密文。（下面的1~7分别将明文保存到哪些节点）
//  1: P0,
//  2: P1,
//  4: P2,
//  3: P0 和 P1
//  5: P0 和 P2
//  6: P1 和 P2
//  7: P0, P1 和 P2
```


<br/>

本节教程中，我们使用 `rosetta` 进行模型的训练，然后将模型保存为`明文`，接着将这个`明文模型`加载到 `tensorflow` 版本进行预测，最后看看与直接使用 `tensorflow` 进行明文训练然后预测之间的误差如何。

<br/>

我们在之前写的 `rosetta` 版本的基础上，加入一些 `save` 相关的代码。

在训练开始之前

```py
# save
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')
os.makedirs("./log/ckpt"+str(party_id), exist_ok=True)
```

在训练结束之后

```py
saver.save(sess, './log/ckpt'+str(party_id)+'/model')
```

具体加在什么位置，可以参考完整代码 [rtt-linear_regression_saver.py](../example/tutorials/code/rtt-linear_regression_saver.py)。


执行 

```sh
./tutorials.sh rtt linear_regression_saver
```

输出如下：

```sh
...
```


#### 模型加载与预测

上一步已经（根据配置文件）将模型保存到相应的节点了，现在直接使用 `tensorflow` 加载上一步保存的明文模型，进行预测。


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


完整代码参考 [tf-linear_regression_restore.py](../example/tutorials/code/tf-linear_regression_restore.py)。

执行 

```sh
./tutorials.sh tf linear_regression_restore
```

输出如下：

```log
Y_pred: [[5.4112522 ]
 [5.80601873]
 [5.96399414]
 ...
 [4.97999231]
 [5.97734902]
 [5.98777173]]
```

<br/>

汇总：

完整代码列表参考

tensorflow 版本

|                |                                                                                          |
| -------------- | ---------------------------------------------------------------------------------------- |
| 基础           | [tf-linear_regression.py](../example/tutorials/code/tf-linear_regression.py)             |
| 模型加载与预测 | [tf-linear_regression_saver.py](../example/tutorials/code/tf-linear_regression_saver.py) |
| 评估           | [tf-linear_regression_stat.py](../example/tutorials/code/tf-linear_regression_stat.py)   |

rosetta 版本

|                  |                                                                                              |
| ---------------- | -------------------------------------------------------------------------------------------- |
| 基础             | [rtt-linear_regression.py](../example/tutorials/code/rtt-linear_regression.py)               |
| 基础（输出明文） | [rtt-linear_regression_reveal.py](../example/tutorials/code/rtt-linear_regression_reveal.py) |
| 模型训练与保存   | [rtt-linear_regression_saver.py](../example/tutorials/code/rtt-linear_regression_saver.py)   |
| 评估             | [rtt-linear_regression_stat.py](../example/tutorials/code/rtt-linear_regression_stat.py)     |

### 逻辑回归

有了上面线性回归的基础，那`逻辑回归（Logistic Regression）`就`非常非常非常`简单了。

在线性回归的基础上，我们使用 `sigmoid` 做为二分类器，使用`交叉熵`做为损失函数，构建一个逻辑回归的模型。

无论是 `tensorflow` 版本，还是 `rosetta` 版本，改动的地方是一样的，对比 `线性回归` 版本，只需要改动模型的构建，即仅仅只需要进行如下改动即可：

- 预测值，加上 `sigmoid`

- 损失函数使用交叉熵

```py
# predict
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)

# loss
logits = tf.matmul(X, W) + b
loss = tf.nn.sigmoid_cross_entropy_with_logits(labels=Y, logits=logits)
loss = tf.reduce_mean(loss)
```

完整代码列表参考

tensorflow 版本

|                |                                                                                                  |
| -------------- | ------------------------------------------------------------------------------------------------ |
| 基础           | [tf-logistic_regression.py](../example/tutorials/code/tf-logistic_regression.py)                 |
| 模型训练与保存 | [tf-logistic_regression_saver.py](../example/tutorials/code/tf-logistic_regression_saver.py)     |
| 模型加载与预测 | [tf-logistic_regression_restore.py](../example/tutorials/code/tf-logistic_regression_restore.py) |
| 评估           | [tf-logistic_regression_stat.py](../example/tutorials/code/tf-logistic_regression_stat.py)       |

rosetta 版本

|                      |                                                                                                    |
| -------------------- | -------------------------------------------------------------------------------------------------- |
| 基础                 | [rtt-logistic_regression.py](../example/tutorials/code/rtt-logistic_regression.py)                 |
| 基础（输出明文）     | [rtt-logistic_regression_reveal.py](../example/tutorials/code/rtt-logistic_regression_reveal.py)   |
| 模型训练与保存       | [rtt-logistic_regression_saver.py](../example/tutorials/code/rtt-logistic_regression_saver.py)     |
| 模型(密文)加载与预测 | [rtt-logistic_regression_restore.py](../example/tutorials/code/rtt-logistic_regression_restore.py) |
| 评估                 | [rtt-logistic_regression_stat.py](../example/tutorials/code/rtt-logistic_regression_stat.py)       |



执行方式与 `线性回归` 是一样的。


<br/>

此处展示一下逻辑回归的评估对比。


tensorflow:

```json
{
  "tag": "tensorflow",
  "score_auc": 0.7346938775510203,
  "score_ks": 0.428171268507403,
  "threshold_opt": 0.6036468147999999,
  "score_accuracy": 0.71,
  "score_precision": 0.8666666666666667,
  "score_recall": 0.5098039215686274,
  "score_f1": 0.6419753086419753
}
```
rosetta:

```json
{
  "tag": "rosetta",
  "score_auc": 0.7366946778711485,
  "score_ks": 0.42737094837935174,
  "threshold_opt": 0.6110839844,
  "score_accuracy": 0.71,
  "score_precision": 0.84375,
  "score_recall": 0.5294117647058824,
  "score_f1": 0.6506024096385543
}
```

> 此处只需要关心两个版本之间的误差（是非常小的）

> rosetta 甚至比 tensorflow 稍微好一些

下图是关于 `tensorflow` 与 `rosetta` 预测值的误差对比。

![](./_static/tutorials/logistic_regression_stat-Y-diff4.png)


### 支持超大数据集

以上的线性回归、逻辑回归模型都是把数据集全部加载到内存中，然后依次按批量取出来进行训练，随着数据集规模越来越大，一次性把数据集加载到内存已经变的不现实。

TensorFlow 等主流明文 AI 框架已经意识并提供解决方案，TensorFlow 中提供相关的 Dataset APIs 来构建低内存消耗的、复杂的、可复用的数据管道，由于 Rosetta 使用 TensorFlow 作为后端，因此稍微修改即可复用。

我们使用逻辑回归模型作为例子来说明如何使用大数据集进行训练。

TensorFlow 完整代码参考 [tf-ds-lr.py](../example/tutorials/code/tf-ds-lr.py) 。

Rosetta 完整代码参考 [rtt-ds-lr.py](../example/tutorials/code/rtt-ds-lr.py)。

仔细分析 tf-ds-lr.py 和 rtt-ds-lr.py 中的代码，主要有两个不同点：
1. 创建文本行数据集，TensorFlow 中使用 TextLineDataset 类，而 Rosetta 中使用 PrivateTextLineDataset 类。
    TensorFlow 中代码如下：
    ```py
    dataset_x = tf.data.TextLineDataset(file_x)
    ...
    ```
    Rosetta 中代码如下：
    ```py
    dataset_x0 = rtt.PrivateTextLineDataset(
                    file_x, data_owner=0)  # P0 hold the file data
    ...
    ```

2. Decode 函数实现不一样，TensorFlow 版本中 Decode 函数中把行筛分为对应的字段后，然后把筛分后的字段转换为数值，而 Rosetta 版本中的 Decode 函数首先也是把行筛分为对应的字段后，然后调用 `PrivateInput` 进行数据分享。
    TensorFlow 中代码如下：
    ```py
    # dataset decode
    def decode_x(line):
        fields = tf.string_split([line], ',').values
        fields = tf.string_to_number(fields, tf.float64)
        return fields
    ```
    Rosetta 中代码如下：
    ```py
    # dataset decode
    def decode_p0(line):
        fields = tf.string_split([line], ',').values
        fields = rtt.PrivateInput(fields, data_owner=0) # P0 hold the file data
        return fields
    ```

## 结语

OK，你现在已经完全掌握了 `Rosetta` 的使用了，赶紧找一个真实场景玩玩。

欢迎!


## 附加


### 数据集说明

数据集来源参考[这里](http://archive.ics.uci.edu/ml/datasets/Wine+Quality)。

我们将其做了简单的处理如下，存放路径在 `dsets/`，目录结构如下

```
dsets/
├── ALL
│   ├── cls_test_x.csv
│   ├── cls_test_y.csv
│   ├── cls_train_x.csv
│   ├── cls_train_y.csv
│   ├── reg_test_x.csv
│   ├── reg_test_y.csv
│   ├── reg_train_x.csv
│   └── reg_train_y.csv
├── P0
│   ├── cls_test_x.csv
│   ├── cls_test_y.csv
│   ├── cls_train_x.csv
│   ├── cls_train_y.csv
│   ├── reg_test_x.csv
│   └── reg_train_x.csv
├── P1
│   ├── cls_test_x.csv
│   ├── cls_train_x.csv
│   ├── reg_test_x.csv
│   ├── reg_test_y.csv
│   ├── reg_train_x.csv
│   └── reg_train_y.csv
└── P2
```

|        |                                |
| ------ | ------------------------------ |
| ALL    | 数据集的原始数据               |
| P*     | 表示各节点拥有的私有数据       |
| cls*   | 表示二分类数据集，用于逻辑回归 |
| reg*   | 表示回归数据集，用于线性回归   |
| *train | 表示用于训练的数据集           |
| *test  | 表示用于预测的数据集           |
| *x     | 表示样本                       |
| *y     | 表示标签                       |

说明：

为了与明文（tensorflow 版本）对比，我们把原始数据集按垂直方向切分成两份，一份当做是 `P0` 的私有数据，另一分当做是 `P1` 的私有数据。

1. ALL 下的数据用于 tensorflow 版本。
2. P0/P1 各节点的私有数据存放在各自的节点上。
3. P2 是没有数据的。
4. 逻辑回归的标签由 P0 执有，线性回归的标签由 P1 执有。

