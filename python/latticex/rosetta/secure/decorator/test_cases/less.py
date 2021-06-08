#!/usr/bin/python

import latticex.rosetta as rtt

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
print("=========================== tf op less 1")
xc = tf.less(xa, xb)
xcc = sess.run(xc)
print("=========================== tf op less 2")
print(xcc)

print("=========================== mpc op less 1")
xc = rtt.SecureLess(xa, xb)
revealed_res = rtt.SecureReveal(xc)
xcc, revealed_res = sess.run([xc, revealed_res])
print("Ciphertext res: ", xcc)
print("Plaintext res: ", revealed_res)
print("=========================== mpc op less 2")

###########
