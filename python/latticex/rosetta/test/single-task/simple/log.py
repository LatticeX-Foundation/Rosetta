#!/usr/bin/python

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf
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
print("=========================== tf op log 1")
xc = tf.log(xa)
xcc = sess.run(xc)
print("=========================== tf op log 2")
print("TF Log: ", xcc)

import latticex.rosetta as cb
# cb.py_protocol_handler.set_loglevel(0)
cb.activate("SecureNN")
xb = tf.Variable(
    [
        [1.892, 2],
        [3, 4.43],
        [.0091, .3]
    ]
)

print("=========================== secure op log 1")
#xc = cb.SecureAbs(xb)
xc = cb.SecureLog(xb)
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("MPC cipher Log:", sess.run(xc))
xcc = sess.run(cb.SecureReveal(xc))
print("MPC plain Log: ", xcc)
print("=========================== secure op log 2")
print("=========================== secure op high-precision log 1")
xc = cb.SecureHLog(xb)
xcc = sess.run(cb.SecureReveal(xc))
cb.deactivate()
print("MPC plain HLog: ", xcc)
print("=========================== secure op high-precision log 2")
###########: