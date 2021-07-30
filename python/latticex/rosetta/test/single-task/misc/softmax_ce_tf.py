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
labels = [0, 1, 0]
res1 = tf.nn.softmax(logits)
res2 = tf.log(res1)
res3 = tf.nn.softmax_cross_entropy_with_logits(logits=logits, labels=labels)

with tf.Session() as sess:
    res1, res2, res3 = sess.run([res1, res2, res3])
    print('res1:', res1)
    print('res2:', res2)
    print('res3:', res3)
print('====================================================')

###############################
# multi-samples
###############################
logits = tf.constant([[2, 7, 5], [6, 3, 4]], dtype=tf.float32)
labels = [[0, 1, 0], [1, 0, 0]]
res1 = tf.nn.softmax(logits)
res2 = tf.log(res1)
res3 = tf.nn.softmax_cross_entropy_with_logits(logits=logits, labels=labels)
res4 = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(
    logits=logits, labels=labels))

with tf.Session() as sess:
    res1, res2, res3, res4 = sess.run([res1, res2, res3, res4])
    print('res1:', res1)
    print('res2:', res2)
    print('res3:', res3)
    print('res4:', res4)
