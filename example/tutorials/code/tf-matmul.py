#!/bin/python3
# Example of Matmul for TensorFlow
import tensorflow as tf
import numpy as np
x = tf.Variable([[1, 2], [2, 3]])
y = tf.Variable([[1, 2], [2, 3]])
res = tf.matmul(x, y)
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    res = sess.run(res)
    print('tf.matmul:', sess.run(res))  # ret: 1.0
    print('numpy.matmul:', np.matmul([[1, 2], [2, 3]], [[1, 2], [2, 3]]))
