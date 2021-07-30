#!/usr/bin/python
# coding:utf-8
import latticex.rosetta as rtt
import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)
tf.set_random_seed(0)


protocol="SecureNN"
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")
rtt.activate(protocol)

print("rtt.get_protocol_name():", rtt.get_protocol_name())
patyid = rtt.get_party_id("")

###############################
# 1-d
###############################
print('=================1-d=========================')
x = [1, 8, 3]
print(x)

a = tf.Variable(x, dtype=tf.float64)
aa = tf.reduce_mean(a)
a0 = tf.reduce_mean(a, axis=0)

aa_k = tf.reduce_mean(a, keepdims=True)
a0_k = tf.reduce_mean(a, axis=0, keepdims=True)

with tf.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    res_aa = sess.run(rtt.SecureReveal(aa))
    print('tf tf.reduce_mean aa:\n', res_aa)
    res_a0 = sess.run(rtt.SecureReveal(a0))
    print('tf tf.reduce_mean a0:\n', res_a0)

    print('====================================================')
    res_aa_k = sess.run(rtt.SecureReveal(aa_k))
    print('tf tf.reduce_mean aa_k:\n', res_aa_k)
    res_a0_k = sess.run(rtt.SecureReveal(a0_k))
    print('tf tf.reduce_mean a0_k:\n', res_a0_k)
print('====================================================')


###############################
# 2-d
###############################
print('=================2-d=========================')
x = [[1, 4, 2],
     [0, 5, 3]]
print(x)

a = tf.Variable(x, dtype=tf.float64)
aa = tf.reduce_mean(a)
a0 = tf.reduce_mean(a, axis=0)
a1 = tf.reduce_mean(a, axis=1)

aa_k = tf.reduce_mean(a, keepdims=True)
a0_k = tf.reduce_mean(a, axis=0, keepdims=True)
a1_k = tf.reduce_mean(a, axis=1, keepdims=True)

with tf.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    res_aa = sess.run(rtt.SecureReveal(aa))
    print('tf tf.reduce_mean aa:\n', res_aa)
    res_a0 = sess.run(rtt.SecureReveal(a0))
    print('tf tf.reduce_mean a0:\n', res_a0)
    res_a1 = sess.run(rtt.SecureReveal(a1))
    print('tf tf.reduce_mean a1:\n', res_a1)

    print('====================================================')
    res_aa_k = sess.run(rtt.SecureReveal(aa_k))
    print('tf tf.reduce_mean aa_k:\n', res_aa_k)
    res_a0_k = sess.run(rtt.SecureReveal(a0_k))
    print('tf tf.reduce_mean a0_k:\n', res_a0_k)
    res_a1_k = sess.run(rtt.SecureReveal(a1_k))
    print('tf tf.reduce_mean a1_k:\n', res_a1_k)
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

a = tf.Variable(x, dtype=tf.float64)
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
    res_aa = sess.run(rtt.SecureReveal(aa))
    print('tf tf.reduce_mean aa:\n', res_aa)
    res_a0 = sess.run(rtt.SecureReveal(a0))
    print('tf tf.reduce_mean a0:\n', res_a0)
    res_a1 = sess.run(rtt.SecureReveal(a1))
    print('tf tf.reduce_mean a1:\n', res_a1)
    res_a2 = sess.run(rtt.SecureReveal(a2))
    print('tf tf.reduce_mean a2:\n', res_a2)
    res_a01 = sess.run(rtt.SecureReveal(a01))
    print('tf tf.reduce_mean a01:\n', res_a01)
    res_a02 = sess.run(rtt.SecureReveal(a02))
    print('tf tf.reduce_mean a02:\n', res_a02)
    res_a12 = sess.run(rtt.SecureReveal(a12))
    print('tf tf.reduce_mean a12:\n', res_a12)

    print('====================================================')
    res_aa_k = sess.run(rtt.SecureReveal(aa_k))
    print('tf tf.reduce_mean aa_k:\n', res_aa_k)
    res_a0_k = sess.run(rtt.SecureReveal(a0_k))
    print('tf tf.reduce_mean a0_k:\n', res_a0_k)
    res_a1_k = sess.run(rtt.SecureReveal(a1_k))
    print('tf tf.reduce_mean a1_k:\n', res_a1_k)
    res_a2_k = sess.run(rtt.SecureReveal(a2_k))
    print('tf tf.reduce_mean a2_k:\n', res_a2_k)
    res_a01_k = sess.run(rtt.SecureReveal(a01_k))
    print('tf tf.reduce_mean a01_k:\n', res_a01_k)
    res_a02_k = sess.run(rtt.SecureReveal(a02_k))
    print('tf tf.reduce_mean a02_k:\n', res_a02_k)
    res_a12_k = sess.run(rtt.SecureReveal(a12_k))
    print('tf tf.reduce_mean a12_k:\n', res_a12_k)
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
a = tf.Variable(x, dtype=tf.float64)
aa = tf.reduce_mean(a)
a0 = tf.reduce_mean(a, axis=0)
a1 = tf.reduce_mean(a, axis=1)
a2 = tf.reduce_mean(a, axis=2)
a3 = tf.reduce_mean(a, axis=3)
a12 = tf.reduce_mean(a, axis=[1, 2])
a12_k = tf.reduce_mean(a, axis=[1, 2], keep_dims=True)

with tf.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    res_aa = sess.run(rtt.SecureReveal(aa))
    print('tf tf.reduce_mean aa:\n', res_aa)
    res_a0 = sess.run(rtt.SecureReveal(a0))
    print('tf tf.reduce_mean a0:\n', res_a0)
    res_a1 = sess.run(rtt.SecureReveal(a1))
    print('tf tf.reduce_mean a1:\n', res_a1)
    res_a2 = sess.run(rtt.SecureReveal(a2))
    print('tf tf.reduce_mean a2:\n', res_a2)
    res_a3 = sess.run(rtt.SecureReveal(a3))
    print('tf tf.reduce_mean a3:\n', res_a3)
    res_a12 = sess.run(rtt.SecureReveal(a12))
    print('tf tf.reduce_mean a12:\n', res_a12)
    res_a12_k = sess.run(rtt.SecureReveal(a12_k))
    print('tf tf.reduce_mean a12_k:\n', res_a12_k)
print('====================================================')

rtt.deactivate()
