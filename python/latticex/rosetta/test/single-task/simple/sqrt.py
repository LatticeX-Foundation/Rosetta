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
print("=========================== tf op sqrt 1")
xc = tf.sqrt(xa)
xcc = sess.run(xc)
print("=========================== tf op sqrt 2")
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

print("=========================== secure op sqrt 1")
#xc = rtt.SecureAbs(xb)
xc = rtt.SecureSqrt(xb)
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("MPC cipher sqrt:", sess.run(xc))
xcc = sess.run(rtt.SecureReveal(xc))
print("MPC plain sqrt: ", xcc)
#########################################
rtt.deactivate()