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
# deal with : ssl.SSLCertVerificationError: [SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed: ...
ssl._create_default_https_context = ssl._create_unverified_context

np.set_printoptions(suppress=True)
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
np.random.seed(0)

EPOCHES = 100
BATCH_SIZE = 256
learning_rate = 0.001

# real data
# ########################################
# Note: We set one_hot so that the 10 labels are vectorized as 0 or 1.
mnist = input_data.read_data_sets("/tmp/data/", one_hot=True)
train_images = mnist.train.images
train_labels = mnist.train.labels
test_images = mnist.test.images
test_labels = mnist.test.labels

print("train_images_shape:", train_images.shape)
print("train_labels_shape:", train_labels.shape)
print("test_images_shape:", test_images.shape)
print("test_labels_shape:", test_labels.shape)
real_X, real_Y = train_images, train_labels
seed = 113
np.random.seed(seed)
np.random.shuffle(real_X)
np.random.seed(seed)
np.random.shuffle(real_Y)

# We only use the first one-hot encoded label, such that this is a binary classification task on the first label.
real_y = np.ones([real_Y.shape[0], 1])
for i in range(len(real_Y)):
    if real_Y[i][0] > 0.5:
        real_y[i] = 0
real_Y = real_y

# ########################################
DIM_NUM = real_X.shape[1]

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

# loss
logits = tf.matmul(X, W) + b
loss = tf.nn.sigmoid_cross_entropy_with_logits(labels=Y, logits=logits)
loss = tf.reduce_mean(loss)
# print(loss)

# optimizer
train = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)
# print(train)

init = tf.global_variables_initializer()
# print(init)

saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')
os.makedirs("./log/ckpt0", exist_ok=True)

with tf.Session() as sess:
    sess.run(init)
    # train
    BATCHES = math.ceil(len(real_X) / BATCH_SIZE)
    print("Begin training...")
    for e in range(EPOCHES):
        for i in range(BATCHES):
            bX = real_X[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            bY = real_Y[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            sess.run(train, feed_dict={X: bX, Y: bY})
    saver.save(sess, './log/ckpt0/model')
    print("Training done, and the model is saved in ./log/ckpt0/model .")