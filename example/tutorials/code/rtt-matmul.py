#!/bin/python3
# Example of Matmul for Rosetta
import latticex.rosetta as rtt # import Rosetta
import tensorflow as tf
import numpy as np
rtt.activate("SecureNN") # activate the SecureNN protocol computation exectution environment
x = tf.Variable(rtt.private_input(0, [[1, 2], [2, 3]])) # Alice private_input x
y = tf.Variable(rtt.private_input(1, [[1, 2], [2, 3]])) # Bob private_input y
res = tf.matmul(x, y)
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    res = sess.run(res)
    print('Rosetta.matmul:', sess.run(rtt.SecureReveal(res)))  # ret: [[b'14.000000' b'20.000000'] [b'20.000000' b'29.000000']]
    print('numpy.matmul:', np.matmul([[1, 2], [2, 3]], [[1, 2], [2, 3]]))
