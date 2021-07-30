#!/usr/bin/python
# coding:utf-8

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

tf.set_random_seed(0)

###############################
# single-samples
###############################
logits = tf.constant([2, 7, 5], dtype=tf.float32)
res1 = tf.nn.softmax(logits)

with tf.Session() as sess:
    res1 = sess.run(res1)
    print(' tf res1:', res1)
print('====================================================')

###############################
# multi-samples
###############################
logits = tf.constant([[2, 7, 5], [6, 3, 4]], dtype=tf.float32)
res1 = tf.nn.softmax(logits)
with tf.Session() as sess:
    res1 = sess.run(res1)
    print(' tf res1:', res1)

# a = {0.1, 0.3, 0.5, 0.2, 0.4, 0.6};
a = np.array([0.1, 0.3, 0.5, 0.2, 0.4, 0.6]).reshape(2, 3)
print('a', a)
"""
a [[0.1 0.3 0.5]
 [0.2 0.4 0.6]]
"""
logits = tf.constant([[0.1, 0.3, 0.5], [0.2, 0.4, 0.6]], dtype=tf.float32)
res1 = tf.nn.softmax(logits)
with tf.Session() as sess:
    res1 = sess.run(res1)
    print(' tf res1:', res1)

logits = tf.constant([[0.1, 0.3, 0.5], [0.2, 0.4, 0.6]], dtype=tf.float32)
res1 = tf.nn.softmax(logits, 0)
with tf.Session() as sess:
    res1 = sess.run(res1)
    print(' tf res1:', res1)
