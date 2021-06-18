#!/usr/bin/python

import latticex.rosetta as rtt
# rtt.set_backend_loglevel(1)
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
xc = rtt.SecureAbs(xa)
revealed_abs = rtt.SecureReveal(xc)
xcc, plain_abs = sess.run([xc, revealed_abs])
print("cipher abs res:", xcc)
print("plain abs res:", plain_abs)
print("=========================== mpc op abs 2")


print("=========================== mpc op AbsPrime 1")
xc = rtt.SecureAbsPrime(xa)
revealed_abs_prime = rtt.SecureReveal(xc)
xcc, plain_abs_prime = sess.run([xc, revealed_abs_prime])
print("cipher abs prime res:", xcc)
print("plain abs prime res:", plain_abs_prime)
print("=========================== mpc op AbsPrime 2")
###########
