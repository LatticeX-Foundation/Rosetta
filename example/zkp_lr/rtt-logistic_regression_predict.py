#!/usr/bin/env python3
from tensorflow.examples.tutorials.mnist import input_data
import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import csv
import tensorflow as tf
import pandas as pd
import numpy as np
import time
import argparse
import pandas as pd
from util import *

import ssl
# ssl.SSLCertVerificationError: [SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed: ...
ssl._create_default_https_context = ssl._create_unverified_context


np.set_printoptions(suppress=True)
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
np.random.seed(0)

# ################################################## Args
parser = argparse.ArgumentParser(description="ResNet Model")
parser.add_argument('--input_public', action='store_true',
                    help="input is public?", default=False)
parser.add_argument('--model_public', action='store_true',
                    help="load plain model as public?", default=False)
parser.add_argument('--res_n', type=int, help="50,101,152", default=50)
# For this demo, only mnist is supported
parser.add_argument('--dset', type=str, help="mnist,cifar10", default='mnist')
args, unparsed = parser.parse_known_args()
# ##################################################
print('Is input data public?:', args.input_public)
print('Is the model public?:', args.model_public)


TEST_DATA_SIZE = 10
learning_rate = 0.001

start_time0 = time.time()
start_time = time.time()
protocol = "Mystique"
rtt.activate(protocol)
if args.model_public:
    rtt.set_restore_model(False)
else:
    rtt.set_restore_model(False, plain_model='P0')

mpc_player_id = rtt.py_protocol_handler.get_party_id()
rtt.set_backend_loglevel(3)
print("pystats activate elapse:{0} s".format(
    time.time() - start_time), flush=True)


# ########################################
# Note: We set one_hot so that the 10 labels are vectorized as 0 or 1.
mnist = input_data.read_data_sets("/tmp/data/", one_hot=True)
test_images = mnist.test.images
test_labels = mnist.test.labels

print("test_images_shape:", test_images.shape)
print("test_labels_shape:", test_labels.shape)
real_X, real_Y = test_images, test_labels
seed = 113
np.random.seed(seed)
np.random.shuffle(real_X)
np.random.seed(seed)
np.random.shuffle(real_Y)
# print(real_X.shape)
# print(real_Y.shape)
# print(real_Y[0])

# We only use the first one-hot encoded label, such that this is a binary classification task on the first label.
real_y = np.ones([real_Y.shape[0], 1])
for i in range(len(real_Y)):
    if real_Y[i][0] > 0.5:
        real_y[i] = 0
real_Y = real_y

oreal_X = real_X[:TEST_DATA_SIZE, :]
oreal_Y = real_Y[:TEST_DATA_SIZE, :]
real_X = oreal_X
real_Y = oreal_Y

start_time = time.time()
if args.input_public:
    real_X = rtt.public_input(0, real_X)
    real_Y = rtt.public_input(0, real_Y)
    print("pystats rtt.public_input elapse:{0} s".format(
        time.time() - start_time))
else:
    real_X = rtt.private_input(0, real_X)
    real_Y = rtt.private_input(0, real_Y)
    print("pystats rtt.private_input elapse:{0} s".format(
        time.time() - start_time))
# ########################################
DIM_NUM = real_X.shape[1]
print("DIM_NUM:", DIM_NUM)

start_time = time.time()
X = tf.placeholder(tf.float64, [None, DIM_NUM])
Y = tf.placeholder(tf.float64, [None, 1])
# print(X)
# print(Y)

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64), name='w')
b = tf.Variable(tf.zeros([1], dtype=tf.float64), name='b')
# print(W)
# print(b)

# predict
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)
# print(pred_Y)


# save
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')

# init
init = tf.global_variables_initializer()
reveal_Y = rtt.SecureReveal(pred_Y)
print("pystats create LR model elapse:{0} s".format(time.time() - start_time))

with tf.Session() as sess:
    sess.run(init)
    start_time = time.time()
    if args.model_public or mpc_player_id == 0:
        if os.path.exists('./log/ckpt0/checkpoint'):
            saver.restore(sess, './log/ckpt0/model')
        else:
            print("model file is not exist!")
            exit(0)
    else:
        with open("/tmp/tftmpmodel", 'w') as f:
            pass
        saver.restore(sess, '/tmp/tftmpmodel')
    print("pystats restore elapse:{0} s".format(time.time() - start_time))

    # predict
    start_time = time.time()
    reveal_y = sess.run(reveal_Y, feed_dict={X: real_X})
    print("pystats predict elapse:{0} s".format(time.time() - start_time))
    v_logits = reveal_y.reshape(-1)

    if mpc_player_id == 0:
        savepath = "log/preds_zk_mnist.csv"
        if args.input_public:
            savepath = "log/preds_zk_mnist_logistic_input_public.csv"
        elif args.model_public:
            savepath = "log/preds_zk_mnist_logistic_model_public.csv"
        savecsv(savepath, v_logits.astype(float))

    score = score_logistic_regression(
        reveal_y.astype(float), oreal_Y, tag='zk')
    print(pretty(score))


print(rtt.get_perf_stats(True))
start_time = time.time()
rtt.deactivate()
print("pystats deactivate elapse:{0} s".format(time.time() - start_time))
print("pystats total elapse:{0} s".format(time.time() - start_time0))
