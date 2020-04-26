import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import csv
import tensorflow as tf
import numpy as np
from util import read_dataset

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

EPOCHES = 10
BATCH_SIZE = 16
learning_rate = 0.0002

# real data
# ######################################## difference from tensorflow
file_x = '../dsets/P' + str(rtt.mpc_player.id) + "/reg_train_x.csv"
file_y = '../dsets/P' + str(rtt.mpc_player.id) + "/reg_train_y.csv"
real_X, real_Y = rtt.MpcDataSet(label_owner=1).load_XY(file_x, file_y)
# ######################################## difference from tensorflow
real_X = real_X[:100, :]
real_Y = real_Y[:100, :]
print(real_X)
DIM_NUM = real_X.shape[1]

X = tf.placeholder(tf.float64, [None, DIM_NUM])
Y = tf.placeholder(tf.float64, [None, 1])
print(X)
print(Y)

# initialize W & b
#W = tf.Variable(tf.ones([DIM_NUM, 1], dtype=tf.float64))
#b = tf.Variable(tf.ones([1], dtype=tf.float64))
w0 = np.ones([DIM_NUM, 1])
b0 = np.ones([1])
W = tf.Variable(rtt.private_input(2, w0))
b = tf.Variable(rtt.private_input(2, b0))
print(W)
print(b)

# predict
pred_Y = tf.matmul(X, W) + b
print(pred_Y)

XR = rtt.MpcReveal(X)
YR = rtt.MpcReveal(Y)
RR = rtt.MpcReveal(pred_Y)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    xr = sess.run(RR, feed_dict={X: real_X, Y: real_Y})
    print(xr)
    xr = sess.run(XR, feed_dict={X: real_X})
    print(xr)
    xr = sess.run(YR, feed_dict={Y: real_Y})
    print(xr)

exit(0)
