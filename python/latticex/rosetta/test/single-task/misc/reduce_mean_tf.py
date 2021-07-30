#!/usr/bin/python
# coding:utf-8

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

tf.set_random_seed(0)

###############################
# 1-d
###############################
print('=================1-d=========================')
x = [1, 8, 3]
print(x)

a = tf.constant(x, dtype=tf.float32)
aa = tf.reduce_mean(a)
a0 = tf.reduce_mean(a, axis=0)

aa_k = tf.reduce_mean(a, keepdims=True)
a0_k = tf.reduce_mean(a, axis=0, keepdims=True)

with tf.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    res_aa = sess.run(aa)
    res_a0 = sess.run(a0)
    print('tf tf.reduce_mean aa:\n', res_aa, res_aa.shape)
    print('tf tf.reduce_mean a0:\n', res_a0, res_a0.shape)

    print('====================================================')
    res_aa_k = sess.run(aa_k)
    res_a0_k = sess.run(a0_k)
    print('tf tf.reduce_mean aa_k:\n', res_aa_k, res_aa_k.shape)
    print('tf tf.reduce_mean a0_k:\n', res_a0_k, res_a0_k.shape)
print('====================================================')


###############################
# 2-d
###############################
print('=================2-d=========================')
x = [[1, 4, 2],
     [0, 5, 3]]
print(x)

a = tf.constant(x, dtype=tf.float32)
aa = tf.reduce_mean(a)
a0 = tf.reduce_mean(a, axis=0)
a1 = tf.reduce_mean(a, axis=1)

aa_k = tf.reduce_mean(a, keepdims=True)
a0_k = tf.reduce_mean(a, axis=0, keepdims=True)
a1_k = tf.reduce_mean(a, axis=1, keepdims=True)

with tf.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    res_aa = sess.run(aa)
    res_a0 = sess.run(a0)
    res_a1 = sess.run(a1)
    print('tf tf.reduce_mean aa:\n', res_aa, res_aa.shape)
    print('tf tf.reduce_mean a0:\n', res_a0, res_a0.shape)
    print('tf tf.reduce_mean a1:\n', res_a1, res_a1.shape)

    print('====================================================')
    res_aa_k = sess.run(aa_k)
    res_a0_k = sess.run(a0_k)
    res_a1_k = sess.run(a1_k)
    print('tf tf.reduce_mean aa_k:\n', res_aa_k, res_aa_k.shape)
    print('tf tf.reduce_mean a0_k:\n', res_a0_k, res_a0_k.shape)
    print('tf tf.reduce_mean a1_k:\n', res_a1_k, res_a1_k.shape)
print('====================================================')


###############################
# 3-d
###############################
print('=================3-d=========================')
x = [[[1, 4, 9],
      [0, 6, 3]],
     [[7, 2, 8],
      [1, 5, 3]]]
print(x)

a = tf.constant(x, dtype=tf.float32)
aa = tf.reduce_mean(a)
a0 = tf.reduce_mean(a, axis=0)
a1 = tf.reduce_mean(a, axis=1)
a2 = tf.reduce_mean(a, axis=2)
a01 = tf.reduce_mean(a, axis=[0, 1])
a02 = tf.reduce_mean(a, axis=[0, 2])
a12 = tf.reduce_mean(a, axis=[1, 2])

aa_k = tf.reduce_mean(a, keepdims=True)
a0_k = tf.reduce_mean(a, axis=0, keepdims=True)
a1_k = tf.reduce_mean(a, axis=1, keepdims=True)
a2_k = tf.reduce_mean(a, axis=2, keepdims=True)
a01_k = tf.reduce_mean(a, axis=[0, 1], keepdims=True)
a02_k = tf.reduce_mean(a, axis=[0, 2], keepdims=True)
a12_k = tf.reduce_mean(a, axis=[1, 2], keepdims=True)

with tf.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    res_aa = sess.run(aa)
    res_a0 = sess.run(a0)
    res_a1 = sess.run(a1)
    res_a2 = sess.run(a2)
    res_a01 = sess.run(a01)
    res_a02 = sess.run(a02)
    res_a12 = sess.run(a12)
    print('tf tf.reduce_mean aa:\n', res_aa, res_aa.shape)
    print('tf tf.reduce_mean a0:\n', res_a0, res_a0.shape)
    print('tf tf.reduce_mean a1:\n', res_a1, res_a1.shape)
    print('tf tf.reduce_mean a2:\n', res_a2, res_a2.shape)
    print('tf tf.reduce_mean a01:\n', res_a01, res_a01.shape)
    print('tf tf.reduce_mean a02:\n', res_a02, res_a02.shape)
    print('tf tf.reduce_mean a12:\n', res_a12, res_a12.shape)

    print('====================================================')
    res_aa_k = sess.run(aa_k)
    res_a0_k = sess.run(a0_k)
    res_a1_k = sess.run(a1_k)
    res_a2_k = sess.run(a2_k)
    res_a01_k = sess.run(a01_k)
    res_a02_k = sess.run(a02_k)
    res_a12_k = sess.run(a12_k)
    print('tf tf.reduce_mean aa_k:\n', res_aa_k, res_aa_k.shape)
    print('tf tf.reduce_mean a0_k:\n', res_a0_k, res_a0_k.shape)
    print('tf tf.reduce_mean a1_k:\n', res_a1_k, res_a1_k.shape)
    print('tf tf.reduce_mean a2_k:\n', res_a2_k, res_a2_k.shape)
    print('tf tf.reduce_mean a01_k:\n', res_a01_k, res_a01_k.shape)
    print('tf tf.reduce_mean a02_k:\n', res_a02_k, res_a02_k.shape)
    print('tf tf.reduce_mean a12_k:\n', res_a12_k, res_a12_k.shape)
print('====================================================')


###############################
# 4-d
###############################
print('=================4-d=========================')
npones = np.ones([3, 2, 2, 8])
# print(npones)
# print(npones.shape)

x = npones
x = np.array([[[[1.0, 2, 1, 1, 9, 1, 1, 1],
                [1, 1, 1, 1, 1, 1, 4, 1]],

               [[1, 1, 1, 3, 1, 1, 7, 1],
                [1, 4, 1, 1, 1, 8, 1, 1]]],


              [[[1, 1, 9, 1, 7, 1, 1, 1],
                [1, 2, 1, 1, 1, 1, 1, 6]],

               [[1, 1, 1, 1, 1, 1, 9, 1],
                  [1, 1, 0, 1, 6, 1, 1, 1]]],


              [[[1, 4, 1, 7, 1, 1, 7, 1],
                [1, 1, 0, 1, 1, 8, 1, 1]],

               [[2, 1, 1, 1, 1, 1, 1, 5],
                  [1, 2, 1, 4, 1, 3, 1, 1]]]])


print(x)
a = tf.constant(x, dtype=tf.float32)
aa = tf.reduce_mean(a)
a0 = tf.reduce_mean(a, axis=0)
a1 = tf.reduce_mean(a, axis=1)
a2 = tf.reduce_mean(a, axis=2)
a3 = tf.reduce_mean(a, axis=3)
a12 = tf.reduce_mean(a, axis=[1, 2])
a12_k = tf.reduce_mean(a, axis=[1, 2], keep_dims=True)

with tf.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    res_aa = sess.run(aa)
    res_a0 = sess.run(a0)
    res_a1 = sess.run(a1)
    res_a2 = sess.run(a2)
    res_a3 = sess.run(a3)
    res_a12 = sess.run(a12)
    res_a12_k = sess.run(a12_k)
    print('tf tf.reduce_mean aa:\n', res_aa, res_aa.shape)
    print('tf tf.reduce_mean a0:\n', res_a0, res_a0.shape)
    print('tf tf.reduce_mean a1:\n', res_a1, res_a1.shape)
    print('tf tf.reduce_mean a2:\n', res_a2, res_a2.shape)
    print('tf tf.reduce_mean a3:\n', res_a3, res_a3.shape)
    print('tf tf.reduce_mean a12:\n', res_a12, res_a12.shape)
    print('tf tf.reduce_mean a12_k:\n', res_a12_k, res_a12_k.shape)
print('====================================================')
