import latticex.rosetta as rtt  # difference from tf
import os
import csv
import tensorflow as tf
import numpy as np
from util import read_dataset
import argparse

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

# ################################################## Args
parser = argparse.ArgumentParser(description="Rosetta Linear Regression")
parser.add_argument('--dims', type=int, help="dims", default=5)
parser.add_argument('--epochs', type=int, help="epochs", default=10)
parser.add_argument('--party_id', type=int, help="Party ID")
args, unparsed = parser.parse_known_args()
# ##################################################
EPOCHES = args.epochs
DIM_NUM = args.dims

learning_rate = 0.0002
BATCH_SIZE = 5
DIS_STEP = 5

# real data (for test, use an option to distinguish)
# ######################################## difference from tf
my_party_id = args.party_id

if my_party_id == 2:
    my_party_id = 0

prefix = '../playground/datasets/'
file_name_prefix = prefix + str(DIM_NUM) + "D/" + str(DIM_NUM) + "d"
file_name_suffix = "share_" + str(my_party_id) + ".csv"
real_X = read_dataset(file_name_prefix + "_attr_" + file_name_suffix)
real_Y = read_dataset(file_name_prefix + "_label_" + file_name_suffix)
# ######################################## difference from tf
real_X = np.array(real_X)
real_Y = np.array(real_Y)

X = tf.placeholder(tf.float64, real_X.shape)
Y = tf.placeholder(tf.float64, real_Y.shape)
print(X)
print(Y)

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64))
b = tf.Variable(tf.zeros([1], dtype=tf.float64))
print(W)
print(b)

# predict
pred_Y = tf.matmul(X, W) + b
print(pred_Y)

# loss
loss = tf.reduce_mean(tf.square(Y - pred_Y))
print(loss)

# optimizer
train = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)
print(train)

init = tf.global_variables_initializer()
print(init)

feed = {X: real_X, Y: real_Y}

# ########### for test, reveal
reveal_W = rtt.MpcReveal(W)
reveal_b = rtt.MpcReveal(b)
reveal_Y = rtt.MpcReveal(pred_Y)
# ########### for test, reveal

with tf.Session() as sess:
    sess.run(init)
    #xW, xb = sess.run([W, b])
    #print("init weight:{} \nbias:{}".format(xW, xb))

    rW, rb = sess.run([reveal_W, reveal_b])
    print("init weight:{} \nbias:{}".format(rW, rb))

    for i in range(EPOCHES):
        sess.run(train, feed_dict=feed)

        #xW, xb = sess.run([W, b])
        #print("i:{} weight:{} \nbias:{}".format(i, xW, xb))

        if i % 10 == 0 or (i == EPOCHES-1 and i % 10 != 0):
            rW, rb = sess.run([reveal_W, reveal_b])
            print("i:{:0>4d} weight:{} \nbias:{}".format(i, rW, rb))

    # predict
    Y_pred = sess.run(reveal_Y, feed_dict=feed)
    print("Y_pred:", Y_pred)
