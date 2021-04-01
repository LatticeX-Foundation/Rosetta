#!/usr/bin/python

import latticex.rosetta as rtt

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


xa = tf.Variable(
    [
        [0.123456780, -0.12345678],
        [-0.234567890, 0.34567890]
    ]
)
print("xa:\n", xa)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("Input Xa:", sess.run(xa*2))
###########
print("=========================== tf op relu 1")
xc = tf.nn.relu(xa*2)
xcc = sess.run(xc)
print("=========================== tf op relu 2")
print(xcc)

print("=========================== mpc op relu 1")
xc = rtt.SecureReveal(rtt.SecureRelu(xa))
xcc = sess.run(xc)
print("=========================== mpc op relu 2")
print(xcc)

print("=========================== mpc op reluPrime 1")
xc = rtt.SecureReveal(rtt.SecureReluPrime(xa))
xcc = sess.run(xc)
print("=========================== mpc op reluPrime 2")
print(xcc)
###########
