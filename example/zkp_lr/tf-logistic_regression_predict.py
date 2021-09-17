#!/usr/bin/env python3
from tensorflow.examples.tutorials.mnist import input_data
import math
import os
import csv
import tensorflow as tf
import numpy as np
import pandas as pd
from util import *

import ssl
# ssl.SSLCertVerificationError: [SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed: ...
ssl._create_default_https_context = ssl._create_unverified_context


np.set_printoptions(suppress=True)
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
np.random.seed(0)

TEST_DATA_SIZE = 10

# real data
# ########################################
mnist = input_data.read_data_sets("/tmp/data/", one_hot=True)
train_images = mnist.train.images
train_labels = mnist.train.labels
test_images = mnist.test.images
test_labels = mnist.test.labels

print("train_images_shape:", train_images.shape)
print("train_labels_shape:", train_labels.shape)
print("test_images_shape:", test_images.shape)
print("test_labels_shape:", test_labels.shape)

real_X, real_Y = test_images, test_labels
seed = 113
np.random.seed(seed)
np.random.shuffle(real_X)
np.random.seed(seed)
np.random.shuffle(real_Y)

real_y = np.ones([real_Y.shape[0], 1])
for i in range(len(real_Y)):
    if real_Y[i][0] > 0.5:
        real_y[i] = 0
real_Y = real_y
real_X = real_X[:TEST_DATA_SIZE, :]
real_Y = real_Y[:TEST_DATA_SIZE, :]
# exit(0)
# ########################################
DIM_NUM = real_X.shape[1]

X = tf.placeholder(tf.float64, [None, DIM_NUM])
Y = tf.placeholder(tf.float64, [None, 1])

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64), name='w')
b = tf.Variable(tf.zeros([1], dtype=tf.float64), name='b')

# predict
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)
# print(pred_Y)

# For restoring the trained model
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')

init = tf.global_variables_initializer()

# restore plain model and predict
with tf.Session() as sess:
    sess.run(init)
    if os.path.exists("./log/ckpt0/checkpoint"):
        saver.restore(sess, './log/ckpt0/model')

    # predict
    Y_pred = sess.run(pred_Y, feed_dict={X: real_X})
    # print("Y_pred:", Y_pred)
    v_preds = Y_pred.reshape(-1)
    # save the plaintext prediction result for each testing sample
    savecsv("log/preds_tf_mnist.csv", v_preds)
    score = score_logistic_regression(Y_pred, real_Y, tag='tf')
    print(pretty(score))
