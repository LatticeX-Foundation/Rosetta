#!/usr/bin/python

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

xa = tf.Variable(
    [
        [1, 2],
        [4, 0.5],
        [1, .9]
    ]
)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("input xa:", sess.run(xa))

###########
print("=========================== tf op rsqrt 1")
xc = tf.rsqrt(xa)
xcc = sess.run(xc)
print("=========================== tf op rsqrt 2")
print("TF Rsqrt: ", xcc)

import latticex.rosetta as rtt
# rtt.py_protocol_handler.set_loglevel(0)
rtt.activate("SecureNN")
xb = tf.Variable(
    [
        [1, 2],
        [4, 0.5],
        [1, .9]
    ]
)

print("=========================== secure op rsqrt 1")
#xc = rtt.SecureAbs(xb)
xc = rtt.SecureRsqrt(xb)
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("MPC cipher rsqrt:", sess.run(xc))
xcc = sess.run(rtt.SecureReveal(xc))
print("MPC plain rsqrt: ", xcc)
#########################################
rtt.deactivate()