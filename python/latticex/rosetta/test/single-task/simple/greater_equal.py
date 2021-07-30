#!/usr/bin/python

import latticex.rosetta as cb

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


xa = tf.Variable(
    [
        [1.892, 2],
        [-2.3, 4.43],
        [.0091, .3]
    ]
)
xb = tf.Variable(
    [
        [2.892, 2],
        [-2.3, 4.43],
        [.0091, -0.3]
    ]
)

print("xa:\n", xa)
print("xb:\n", xb)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

###########
print("=========================== tf op greater_equal 1")
xc = tf.greater_equal(xa, xb)
xcc = sess.run(xc)
print("=========================== tf op greater_equal 2")
print(xcc)

print("=========================== mpc op greater_equal 1")
xc = cb.SecureGreaterEqual(xa, xb)
xcc = sess.run(xc)
print("=========================== mpc op greater_equal 2")
print(xcc)

###########
