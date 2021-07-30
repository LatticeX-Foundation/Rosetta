import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import sys
import csv
import tensorflow as tf
import numpy as np
import time
import argparse


# rtt.set_backend_loglevel(0)
rtt.activate("SecureNN")
X = tf.Variable([[2., 3.], [4., 5.]], tf.float64, name="x")
Y = tf.Variable([[1.], [1.]], tf.float64, name="y")
W = tf.Variable(tf.zeros([2, 1], dtype=tf.float64), name="w")

# predict
pred = tf.matmul(X, W)
pred_Y = tf.sigmoid(pred)

# backward op
dy = pred_Y - Y
dw = tf.matmul(X, dy, transpose_a=True) 
train = [dy, dw]

# init
init = tf.global_variables_initializer()

try:
    with tf.Session() as sess:
        sess.run(init)

        # train
        sess.run(train)

    print("Pass")
except:
    print("Fail")


rtt.deactivate()

Writer = tf.summary.FileWriter("log/graph_opt", tf.get_default_graph())
Writer.close()