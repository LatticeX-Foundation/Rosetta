#!/usr/bin/python

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
print("=========================== tf op equal 1")
xc = tf.equal(xa, xb)
xcc = sess.run(xc)
print("=========================== tf op equal 2")
print(xcc)

###########
print("=========================== tf op not_equal 1")
xc = tf.not_equal(xa, xb)
xcc = sess.run(xc)
print("=========================== tf op not_equal 2")
print(xcc)

import latticex.rosetta as cb
cb.activate("SecureNN")

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

init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("=========================== mpc op equal 1")
xc = cb.SecureEqual(xa, xb)
xcc = sess.run(xc)
print("=========================== mpc op equal 2")
print(xcc)
xcc = sess.run(cb.SecureReveal(xc))
print("MPC plain Equal: ", xcc)

print("=========================== mpc op not_equal 1")
xc = cb.SecureNotEqual(xa, xb)
xcc = sess.run(xc)
print("=========================== mpc op SecureNotEqual 2")
print(xcc)
xcc = sess.run(cb.SecureReveal(xc))
print("MPC plain NotEqual: \n", xcc)
###########
