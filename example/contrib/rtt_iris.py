import os
import math
import matplotlib.pyplot as plt
import tensorflow as tf
import numpy as np
import pandas as pd

import latticex.rosetta as rtt

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

EPOCHES = 1
BATCH_SIZE = 32
learning_rate = 0.003

rtt.activate("SecureNN")
mpc_player_id = rtt.py_protocol_handler.get_party_id()

# real data
# ######################################## difference from tensorflow
tutorials_path = os.path.abspath(os.path.join(os.getcwd(), "../tutorials/"))
training_features_file_path = tutorials_path + "/dsets/P" + str(mpc_player_id) + "/iris_training_features.csv"
training_label_file_path = tutorials_path + "/dsets/P" + str(mpc_player_id) + "/iris_training_label.csv"
training_features, training_label = rtt.PrivateDataset(data_owner=(0, 1), label_owner=1).load_data(training_features_file_path, training_label_file_path, header=None)
# The mean of label is 1, update label with (label-1) for mean is 0.
# However the open-source RTT now cannot support tf.ones_like() function,
# P0 and P1 must finish this process before multi-party training. 
# Namely data-loading process get the label with mean 0.
# training_label = training_label - tf.ones_like(training_label)
test_features_file_path = tutorials_path + "/dsets/P" + str(mpc_player_id) + "/iris_test_features.csv"
test_label_file_path = tutorials_path + "/dsets/P" + str(mpc_player_id) + "/iris_test_label.csv"
test_features, test_label = rtt.PrivateDataset(data_owner=(0, 1), label_owner=1).load_data(test_features_file_path, test_label_file_path, header=None)
# same preprocessing on label and same reason
# test_label = test_label - tf.ones_like(test_label)
# ######################################## difference from tensorflow
DIM_NUM = training_features.shape[1]

# print(training_label)

X = tf.placeholder(tf.float32, [None, DIM_NUM])
Y = tf.placeholder(tf.float32, [None, 1])
print(X)
print(Y)

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1]))
b = tf.Variable(tf.zeros([1]))
print(W)
print(b)

# predict
pred_Y = tf.matmul(X, W) + b
print(pred_Y)

# loss
loss = tf.square(Y - pred_Y)
loss = tf.reduce_mean(loss)
print(loss)

# optimizer
train = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)
print(train)

init = tf.global_variables_initializer()
print(init)

with tf.Session() as sess:
    sess.run(init)
    xW, xb = sess.run([W, b])
    print("init weight:{} \nbias:{}".format(xW, xb))

    # train
    BATCHES = math.ceil(len(training_features) / BATCH_SIZE)
    for e in range(EPOCHES):
        for i in range(BATCHES):
            bX = training_features[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            bY = training_label[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            sess.run(train, feed_dict={X: bX, Y: bY})

            j = e * BATCHES + i
            if j % 50 == 0 or (j == EPOCHES * BATCHES - 1 and j % 50 != 0):
                xW, xb = sess.run([W, b])
                print("I,E,B:{:0>4d},{:0>4d},{:0>4d} weight:{} \nbias:{}".format(
                    j, e, i, xW, xb))

    # predict
    Y_pred = sess.run(pred_Y, feed_dict={X: test_features, Y: test_label})
    # print("Y_pred:\n", Y_pred)
    # print("Y_true:\n", test_label)
    Y_comp = np.hstack((Y_pred,test_label))
    print("Y_comp:\n", Y_comp)
    # Compute the accuracy.
    # Y_pred_cls0 = np.where(Y_pred >= 0, Y_pred, -1)
    # Y_pred_cls0 = np.where(Y_pred < 0, Y_pred_cls0, 0)
    # Y_pred_cls2 = np.where(Y_pred < 0.05, Y_pred, 1)
    # Y_pred_cls2 = np.where(Y_pred >= 0.05, Y_pred_cls2, 0)
    # Y_pred_cls012 = Y_pred_cls0 + Y_pred_cls2
    # Y_comp = np.hstack((Y_pred_cls012, test_label))
    # print(Y_comp)
    # acc = len([i for i in Y_pred_cls012 if i in test_label]) / len(test_label)
    # print('acc: ', acc)

print(rtt.get_perf_stats(True))
rtt.deactivate()