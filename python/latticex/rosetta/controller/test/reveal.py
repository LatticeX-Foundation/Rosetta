#!/usr/bin/env python3

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

protocol = "Helix"
rtt.activate(protocol)

X = tf.Variable([[1., 1.], [2., 1.]])
Y = tf.Variable([[1., 3.], [1., 1.]])
z = rtt.SecureMatMul(X, Y)

# default receive_party is -1, will output plaintext in P0/P1
zr = rtt.SecureReveal(z)
zr0 = rtt.SecureReveal(z, receive_party=0)  # output plaintext in P0
zr1 = rtt.SecureReveal(z, receive_party=1)  # output plaintext in P1

init = tf.global_variables_initializer()
with tf.Session() as sess1:
    sess1.run(init)
    sess1.run(z)
    print('zzzzzr:', sess1.run(zr))
    print('zzzzzr0:', sess1.run(zr0))
    print('zzzzzr1:', sess1.run(zr1))


X = tf.Variable([[2., 1.8]])
Y = tf.Variable([[1.2, 3.]])
z = rtt.SecureMul(X, Y)

# default receive_party is -1, will output plaintext in P0/P1
zr = rtt.SecureReveal(z)
zr0 = rtt.SecureReveal(z, receive_party=0)  # output plaintext in P0
zr1 = rtt.SecureReveal(z, receive_party=1)  # output plaintext in P1

init = tf.global_variables_initializer()
with tf.Session() as sess1:
    sess1.run(init)
    sess1.run(z)
    print('zzzzzr:', sess1.run(zr))
    print('zzzzzr0:', sess1.run(zr0))
    print('zzzzzr1:', sess1.run(zr1))


exit(0)
