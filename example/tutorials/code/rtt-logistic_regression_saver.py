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

rtt.activate("SecureNN")
mpc_player_id = rtt.py_protocol_handler.get_party_id()

# real data
# ######################################## difference from tensorflow
file_x = '../dsets/P' + str(mpc_player_id) + "/cls_train_x.csv"
file_y = '../dsets/P' + str(mpc_player_id) + "/cls_train_y.csv"
real_X, real_Y = rtt.SecureDataSet(
    label_owner=0).load_XY(file_x, file_y, header=None)
# ######################################## difference from tensorflow
real_X = real_X[:100, :]
real_Y = real_Y[:100, :]
DIM_NUM = real_X.shape[1]

X = tf.placeholder(tf.float64, [None, DIM_NUM])
Y = tf.placeholder(tf.float64, [None, 1])
print(X)
print(Y)

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64))
b = tf.Variable(tf.zeros([1], dtype=tf.float64))
print(W)
print(b)

# predict
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)
print(pred_Y)

# loss
logits = tf.matmul(X, W) + b
loss = tf.nn.sigmoid_cross_entropy_with_logits(labels=Y, logits=logits)
loss = tf.reduce_mean(loss)
print(loss)

# optimizer
train = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)
print(train)

# save
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')
os.makedirs("./log/ckpt"+str(mpc_player_id), exist_ok=True)

# init
init = tf.global_variables_initializer()
print(init)

# ########### for test, reveal
reveal_W = rtt.SecureReveal(W)
reveal_b = rtt.SecureReveal(b)
reveal_Y = rtt.SecureReveal(pred_Y)
# ########### for test, reveal

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

    saver.save(sess, './log/ckpt'+str(mpc_player_id)+'/model')

    # predict
    Y_pred = sess.run(pred_Y, feed_dict={X: real_X, Y: real_Y})
    print("Y_pred:", Y_pred)
