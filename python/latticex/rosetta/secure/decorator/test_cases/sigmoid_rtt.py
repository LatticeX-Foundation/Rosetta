#!/usr/bin/python
# coding:utf-8

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)
tf.set_random_seed(0)

protocol = "SecureNN"
protocol = "Helix"
protocol = "Wolverine"
rtt.activate(protocol)
print("rtt.get_protocol_name():", rtt.get_protocol_name())
patyid = rtt.get_party_id()

###############################
# single-samples
###############################
x = [1.21933774, 1.66604676, 1.26618940, 1.29514586, 1.71182663,
     1.44599081, 1.95028897, 1.29123498, 1.81044357, 1.13242592]
logits = tf.Variable(rtt.private_input(0, x))
res1 = tf.nn.sigmoid(logits)
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    res1 = sess.run(rtt.SecureReveal(res1))
    print('rtt tf.nn.sigmoid res1:\n', res1)

print('====================================================')

rtt.deactivate()
