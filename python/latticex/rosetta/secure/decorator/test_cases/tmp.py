#!/usr/bin/python

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

xa = tf.Variable(
    [
        [3, 30],
        [3, 30],
        [3, 30]
    ]
)
xb = tf.Variable(
    [
        [2,  20],
        [-2, -20],
        [4,  -40]
    ]
)

print("xa:\n", xa*2)
print("xb:\n", xb*2)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("input xa:", sess.run(xa*2))
print("input xb:", sess.run(xb*2))
###########
print("=========================== tf op floordiv 1")
xc = tf.floordiv(xa*2, xb*2)
xcc = sess.run(xc)
print("=========================== tf op floordiv 2")
print(xcc)

print("=========================== tf op divide 1")
xc = tf.divide(xa*2, xb*2)
xcc = sess.run(xc)
print("=========================== tf op divide 2")
print(xcc)

import latticex.rosetta as cb
xa = tf.Variable(
    [
        [3, 30],
        [3, 30],
        [3, 30]
    ]
)
xb = tf.Variable(
    [
        [2,  20],
        [-2, -20],
        [4,  -40]
    ]
)


init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("=========================== mpc op floordiv 1")
xc = cb.SecureFloorDiv(xa, xb)
xcc = sess.run(cb.SecureReveal(xc))
print(xcc)
print("=========================== mpc op floordiv 2")

print("=========================== MPC op divide 1")
xc = cb.SecureDivide(xa, xb)
xcc = sess.run(cb.SecureReveal(xc))
print(xcc)
print("=========================== MPC op divide 2")



print("=========================== mpc op high-precision log 1")
xc = cb.SecureHLog(xa)
xcc = sess.run(cb.SecureReveal(xc))
print("=========================== mpc op high-precision log 2")



print(xcc)
###########:
