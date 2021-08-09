#!/usr/bin/python
# coding:utf-8

import latticex.rosetta as rtt
import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)
tf.set_random_seed(0)

protocol="SecureNN"
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")
rtt.activate(protocol)

print("rtt.get_protocol_name():", rtt.get_protocol_name())
patyid = rtt.get_party_id()

###############################
# single-samples
###############################
x = [2, 7, 5]
logits = tf.Variable(rtt.private_input(0, x))
res1 = tf.nn.softmax(logits)
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    res1 = sess.run(rtt.SecureReveal(res1))
    print('rtt res1', res1)

print('====================================================')

###############################
# multi-samples
###############################
x = [[2, 7, 5], [6, 3, 4]]
logits = tf.Variable(rtt.private_input(0, x))
res1 = tf.nn.softmax(logits)
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    res1 = sess.run(rtt.SecureReveal(res1))
    print('rtt res1', res1)
rtt.deactivate()
