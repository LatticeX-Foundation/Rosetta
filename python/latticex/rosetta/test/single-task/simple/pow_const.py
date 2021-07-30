#!/usr/bin/python

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

x = tf.Variable([[2.0, 3.0, 4.0, 5.0, 6.0, 7.0],
                 [2.0, 3.0, 4.0, 5.0, 6.0, 7.0]])
y = tf.constant([7.0, 6.0, 5.0, 4.0, 3.0, 2.0])
#y = tf.constant([2.0, 2.0, 2.0, 2.0, 2.0, 2.0])

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
z = tf.pow(x, y)
#print("plaintext result:", sess.run(z))

import latticex.rosetta as cb
cb.activate("SecureNN")

x = tf.Variable([[2.0, 3.0, 4.0, 5.0, 6.0, 7.0],
                 [2.0, 3.0, 4.0, 5.0, 6.0, 7.0]])
y = tf.constant([7.0, 6.0, 5.0, 4.0, 3.0, 2.0])
#y = tf.constant([2.0, 2.0, 2.0, 2.0, 2.0, 2.0])
plain_x = cb.SecureReveal(x)
xz = cb.SecurePow(x, y)
rxz = cb.SecureReveal(xz)

init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    print('revealed input X:', sess.run(plain_x))
    expected_res = sess.run(z)
    print('plain result:', expected_res)
    print('cipher result:', sess.run(xz))
    mpc_res = sess.run(tf.strings.to_number(rxz))
    print('revealed result:', mpc_res)
    print("TEST:", mpc_res == expected_res)
