#!/usr/bin/env python3
import math
import os
import sys
import csv
import tensorflow as tf
import numpy as np
import pandas as pd
import time
import argparse

np.set_printoptions(suppress=True)
np.random.seed(0)

EPOCHES = 1
BATCH_SIZE = 32
learning_rate = 0.03125
DIM_NUM = 11
ROW_NUM = 1279

file_x = ""
file_y = ""
filex_name = "cls_train_x.csv"
filey_name = "cls_train_y.csv"

file_x = "../dsets/ALL/" + filex_name
file_y = "../dsets/ALL/" + filey_name

print("file_x:", file_x)
print("file_y:", file_y)
print("DIM_NUM:", DIM_NUM)


# training dataset
dataset_x = tf.data.TextLineDataset(file_x)
dataset_y = tf.data.TextLineDataset(file_y)


# dataset decode
def decode_x(line):
    fields = tf.string_split([line], ',').values
    fields = tf.string_to_number(fields, tf.float64)
    return fields


def decode_y(line):
    fields = tf.string_split([line], ',').values
    fields = tf.string_to_number(fields, tf.float64)
    return fields


# dataset pipeline
dataset_x = dataset_x \
    .map(decode_x) \
    .batch(BATCH_SIZE)

dataset_y = dataset_y \
    .map(decode_y) \
    .batch(BATCH_SIZE)


# make iterator
iter_x = dataset_x.make_initializable_iterator()
X = iter_x.get_next()

iter_y = dataset_y.make_initializable_iterator()
Y = iter_y.get_next()


# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64))
b = tf.Variable(tf.zeros([1], dtype=tf.float64))


# build lr model
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)
dy = pred_Y - Y
dw = tf.matmul(X, dy, transpose_a=True) * (1.0 / BATCH_SIZE)
db = tf.reduce_sum(dy, axis=0) * (1.0 / BATCH_SIZE)
delta_w = dw * learning_rate
delta_b = db * learning_rate
update_w = W - delta_w
update_b = b - delta_b


# update variables
assign_update_w = tf.assign(W, update_w)
assign_update_b = tf.assign(b, update_b)


# training
init = tf.global_variables_initializer()
with tf.Session() as sess:
    # init var & iter
    sess.run(init)
    sess.run([iter_x.initializer, iter_y.initializer])

    # train
    start_time = time.time()
    BATCHES = int(ROW_NUM / BATCH_SIZE)

    for e in range(EPOCHES):
        for i in range(BATCHES):
            sess.run([assign_update_w, assign_update_b])

    training_use_time = time.time() - start_time
    print("training_use_time: {} seconds".format(training_use_time))
