import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import sys
import csv
import tensorflow as tf
import numpy as np
import time
import argparse

rtt.activate("SecureNN")
# rtt.set_backend_loglevel(0)

X = tf.Variable([[2., 3.], [4., 5.]], tf.float64, name="x")
Y = tf.Variable([[1.], [1.]], tf.float64, name="y")
W = tf.Variable(tf.zeros([2, 1], dtype=tf.float64), name="w")
b = tf.Variable(tf.zeros([1], dtype=tf.float64), name="b")

# predict
pred_1 = tf.matmul(X, W)
pred_2 = pred_1 + b
pred_Y = tf.sigmoid(pred_2)

# backward op
alpha = (1.0 / 64)
delta_y = pred_Y - Y
calc_dw = tf.matmul(X, delta_y, transpose_a=True)
dw = calc_dw * alpha
delta_w = dw * 0.01
calc_db = tf.reduce_sum(delta_y, axis=0)
db = calc_db * alpha
delta_b = db * 0.01

train = [tf.assign(W, W - delta_w), tf.assign(b, b - delta_b)]


# ########### for test, reveal
reveal_pred_1 = rtt.SecureReveal(pred_1)
reveal_pred_2 = rtt.SecureReveal(pred_2)
reveal_Y = rtt.SecureReveal(pred_Y)
reveal_delta_y = rtt.SecureReveal(delta_y)
reveal_calc_dw = rtt.SecureReveal(calc_dw)
reveal_dw = rtt.SecureReveal(dw)
reveal_delta_w = rtt.SecureReveal(delta_w)

reveal_calc_db = rtt.SecureReveal(calc_db)
reveal_db = rtt.SecureReveal(db)
reveal_delta_b = rtt.SecureReveal(delta_b)

reveal_W = rtt.SecureReveal(W)
reveal_b = rtt.SecureReveal(b)
# ########### for test, reveal

# init
init = tf.global_variables_initializer()

try:
    with tf.Session() as sess:
        sess.run(init)

        for i in range(5):
            # train
            sess.run(reveal_W)
            sess.run(reveal_b)

            sess.run(reveal_pred_1)
            sess.run(reveal_pred_2)
            sess.run(reveal_Y)
            sess.run(reveal_delta_y)
            sess.run(reveal_calc_dw)
            sess.run(reveal_dw)
            sess.run(reveal_delta_w)
            sess.run(reveal_calc_db)
            sess.run(reveal_db)
            sess.run(reveal_delta_b)
            
            sess.run(train)

    print("Pass")
except:
    print("Fail")


rtt.deactivate()

Writer = tf.summary.FileWriter("log/graph_opt3", tf.get_default_graph())
Writer.close()


