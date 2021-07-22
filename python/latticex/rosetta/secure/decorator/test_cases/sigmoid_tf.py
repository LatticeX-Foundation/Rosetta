#!/usr/bin/python
# coding:utf-8

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

tf.set_random_seed(0)

###############################
# single-samples
###############################
x = [1.21933774, 1.66604676, 1.26618940, 1.29514586, 1.71182663,
     1.44599081, 1.95028897, 1.29123498, 1.81044357, 1.13242592]
x = [0.16156076, -4.10311569, -4.10077932, 0.44367139, 5.75057091,
     9.67613485, -2.37992174, -7.71311783, 0.05045872, -1.29939185]
logits = tf.constant(x, dtype=tf.float32)
res1 = tf.nn.sigmoid(logits)

with tf.Session() as sess:
    res1 = sess.run(res1)
    print('tf tf.nn.sigmoid res1:\n', res1)
print('====================================================')
