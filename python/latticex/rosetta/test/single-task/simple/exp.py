#!/usr/bin/python

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

xa = tf.Variable(
    [
        [1.892, 2],
        [3, 4.43],
        [.0091, .3]
    ]
)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("input xa:", sess.run(xa))

###########
print("=========================== tf op exp 1")
xc = tf.exp(xa)
xcc = sess.run(xc)
print("=========================== tf op exp 2")
print("TF exp: ", xcc)

import latticex.rosetta as rtt
# rtt.set_float_precision(float_precision=16)
# rtt.py_protocol_handler.set_loglevel(0)
rtt.activate("SecureNN")
xb = tf.Variable(
    [
        [1.892, 2],
        [3, 4.43],
        [.0091, .3]
    ]
)

print("=========================== secure op exp 1")
#xc = rtt.SecureAbs(xb)
xc = rtt.SecureExp(xb)
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("MPC cipher exp:", sess.run(xc))
xcc = sess.run(rtt.SecureReveal(xc))
print("MPC plain exp: ", xcc)
print("=========================== secure op exp 2")
###########: