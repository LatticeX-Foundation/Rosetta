#!/usr/bin/python

import latticex.rosetta as rst

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


xa = tf.Variable(
    [
        [-102.12345678],
        [-0.12345678],
        [0.234567890],
        [0.34567890]
    ]
)
print("xa:\n", xa)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("Input Xa:", sess.run(xa*2))
###########
print("=========================== tf op abs 1")
xc = tf.abs(xa)
xcc = sess.run(xc)
print("=========================== tf op abs 2")
print(xcc)

print("=========================== mpc op abs 1")
xc = rst.SecureAbs(xa)
xcc = sess.run(xc)
print("=========================== mpc op abs 2")
print(xcc)

print("=========================== mpc op AbsPrime 1")
xc = rst.SecureAbsPrime(xa)
xcc = sess.run(xc)
print("=========================== mpc op AbsPrime 2")
print(xcc)
###########
rst.deactivate()
