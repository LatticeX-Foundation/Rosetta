#!/usr/bin/env python3
# rosetta LR with sample based (horizonal federated learning)
import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import sys
import csv
import tensorflow as tf
import numpy as np
import pandas as pd
import time
import argparse
from sklearn.metrics import roc_auc_score, roc_curve, f1_score, precision_score, recall_score, accuracy_score

rtt.activate("SecureNN")
mpc_player_id = rtt.py_protocol_handler.get_party_id()
print("mpc_player_id:", mpc_player_id)

np.set_printoptions(suppress=True)
np.random.seed(0)

EPOCHES = 10
BATCH_SIZE = 16
learning_rate = 0.0002
DIM_NUM = 11
ROW_NUM = 1279

file_x = ""
file_y = ""
filex_name = "cls_train_x.csv"
filey_name = "cls_train_y.csv"

file_x = "../dsets/P" + str(mpc_player_id) + "/" + filex_name
file_y = "../dsets/P" + str(mpc_player_id) + "/" + filey_name

print("file_x:", file_x)
print("file_y:", file_y)
print("DIM_NUM:", DIM_NUM)

# If some errors happen, you may remove this temp directory manually.
cache_dir = "./temp{}".format(mpc_player_id)
if not os.path.exists(cache_dir):
    os.makedirs(cache_dir, exist_ok=True)
else:
    # fix TF1.14 cache file bug
    import shutil
    shutil.rmtree(cache_dir)
    os.makedirs(cache_dir, exist_ok=True)

# training dataset
dataset_x0 = rtt.PrivateTextLineDataset(
    file_x, data_owner=0)  # P0 hold the file_x data
dataset_x1 = rtt.PrivateTextLineDataset(
    file_x, data_owner=1)  # P1 hold the file_x data
dataset_y = rtt.PrivateTextLineDataset(
    file_y, data_owner=0)  # P0 hold the file_y data


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
## note that the last batch may fetch elements that are less than BATCH_SIZE
dataset_x0 = dataset_x0 \
    .map(decode_p0)\
    .cache(f"{cache_dir}/cache_p0_x0")\
    .batch(BATCH_SIZE).repeat()

dataset_x1 = dataset_x1 \
    .map(decode_p1)\
    .cache(f"{cache_dir}/cache_p1_x1")\
    .batch(BATCH_SIZE).repeat()

dataset_y = dataset_y \
    .map(decode_p0)\
    .cache(f"{cache_dir}/cache_p0_y")\
    .batch(BATCH_SIZE).repeat()


# make iterator
iter_x0 = dataset_x0.make_initializable_iterator()
X0 = iter_x0.get_next()

iter_x1 = dataset_x1.make_initializable_iterator()
X1 = iter_x1.get_next()

iter_y = dataset_y.make_initializable_iterator()
Y = iter_y.get_next()

# Join input X of P0 and P1, features splitted dataset
X = tf.concat([X0, X1], axis=1)


# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64))
b = tf.Variable(tf.zeros([1], dtype=tf.float64))
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)
revealed_pred_Y = rtt.SecureReveal(pred_Y)

real_Y = rtt.SecureReveal(Y)

###### Build LR model mannually.
# dy = pred_Y - Y
# dw = tf.matmul(X, dy, transpose_a=True) * (1.0 / BATCH_SIZE)
# db = tf.reduce_sum(dy, axis=0) * (1.0 / BATCH_SIZE)
# delta_w = dw * learning_rate
# delta_b = db * learning_rate
# update_w = W - delta_w
# update_b = b - delta_b

# # update variables
# assign_update_w = tf.assign(W, update_w)
# assign_update_b = tf.assign(b, update_b)

###### Build LR model with auto-grad Optimizer
logits = tf.matmul(X, W) + b
loss = tf.nn.sigmoid_cross_entropy_with_logits(labels=Y, logits=logits)
loss = tf.reduce_mean(loss)
train = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)

# training
init = tf.global_variables_initializer()
plain_pred_Y = np.array([])

with tf.Session() as sess:
    # init var & iter
    sess.run(init)
    sess.run([iter_x0.initializer, iter_x1.initializer, iter_y.initializer])

    # train
    start_time = time.time()
    BATCHES = int(ROW_NUM / BATCH_SIZE)

    for e in range(EPOCHES):
        print("{}-th epoch train...".format(e))
        for i in range(BATCHES + 1):
            # sess.run([assign_update_w, assign_update_b])
            sess.run(train)

    training_use_time = time.time()-start_time
    print("training_use_time: {} seconds".format(training_use_time))
    ###### check the prediction result on training dataset itself.
    sess.run([iter_x0.initializer, iter_x1.initializer, iter_y.initializer])

    for i in range(BATCHES + 1):
        batch_revealed_pred_Y = sess.run(revealed_pred_Y)
        plain_pred_Y = np.append(plain_pred_Y, batch_revealed_pred_Y.astype("float"))

print(rtt.get_perf_stats(True))
rtt.deactivate()

if mpc_player_id == 0:
    Y_pred_prob = plain_pred_Y.astype("float").reshape([-1,])
    Y_true = pd.read_csv(file_y, header=None)
    auc_score = roc_auc_score(Y_true, Y_pred_prob)

    Y_pred_class = (Y_pred_prob>=0.5).astype('int64')
    lr_f1_score = f1_score(Y_true, Y_pred_class)
    precision = precision_score(Y_true, Y_pred_class)
    recall = recall_score(Y_true, Y_pred_class)
    accuracy = accuracy_score(Y_true, Y_pred_class)
    print("*************SELF EVALUATION*************")
    print("AUC:", auc_score)
    print("F1_SCORE:", lr_f1_score)
    print("PRECISION:", precision)
    print("RECALL:", recall)
    print("ACCURACY:", accuracy)
    print("*****************************************")

