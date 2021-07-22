#!/usr/bin/python

import time
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

np_a = np.array([
        [-10, -3],
        [10.0, 3.1],
        [-10, -3.1],
        [5.0, 3.1],
        [0.003, -0.02],
        [0.002, -0.02] 
    ], dtype=np.float_)

np_b = np.array([
        [0.0, 0.0], # correct 0
        [0.5, 0.5], # correct 1
        [0.5, 0.5], # wrong 1
        [0.0, 0.0], # wrong 0
        [0.5, 0.5], # hard t0 distinguish 0
        [0.5, 0.5] # hard to distinguish 1
    ], dtype=np.float_)
np_b = np_b * 2

logits = tf.Variable(np_a, dtype=tf.float64)
logits_plain = logits
labels = tf.Variable(np_b, dtype = tf.float64)
labels_plain = labels

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("input Shape:", logits.shape)
print("input logits:", sess.run(logits))
print("input labels:", sess.run(labels))

###########
print("=========================== tf op sigmoid_cross_entropy 1")
result_plain = tf.nn.sigmoid_cross_entropy_with_logits(logits=logits_plain, labels=labels_plain)
xcc = sess.run(result_plain)
print("=========================== tf op sigmoid_cross_entropy 2")
print(xcc)

import latticex.rosetta as rst
print("=========================== mpc op sigmoid_cross_entropy 1")
def test_protocol(protocol_name = "SecureNN"):
    rst.activate(protocol_name)
    PRI_LOGITS = rst.private_input(0, np_a)
    PRI_LABELS = rst.private_input(1, np_b)

    PRI_logits = tf.Variable(PRI_LOGITS, dtype=tf.string)
    PRI_labels = tf.Variable(PRI_LABELS, dtype = tf.string)

    init = tf.compat.v1.global_variables_initializer()
    PRI_sess = tf.compat.v1.Session()
    PRI_sess.run(init)

    start_t = time.time()
    result_mpc = rst.secure_sigmoid_cross_entropy_with_logits(logits=PRI_logits, labels=PRI_labels)
    PRI_sess.run(result_mpc)
    end_t = time.time()
    reveal_op = rst.SecureReveal(result_mpc)
    xcc = PRI_sess.run(reveal_op)
    print(xcc)
    print("{} elapsed: {} ".format(protocol_name, end_t - start_t))
    rst.deactivate()
    
test_protocol("SecureNN")
time.sleep(3)
test_protocol("Helix")

print("=========================== mpc op sigmoid_cross_entropy 2")



