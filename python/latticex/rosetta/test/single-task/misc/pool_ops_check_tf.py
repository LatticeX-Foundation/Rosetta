#!/usr/bin/python

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

tf.set_random_seed(0)



# 1,6,5,3 (NHWC)
"""
  *   C=0         C=1          C=2
  * [0 1 0 1 2 | 1 0 2 1 1 | 2 4 2 2 2]
  * [1 5 4 5 0 | 2 2 0 2 3 | 1 0 5 0 1]
  * [1 5 4 5 0 | 2 0 2 2 3 | 3 0 1 0 2]
  * [1 0 5 0 3 | 2 4 3 0 2 | 0 1 0 0 4]
  * [6 6 6 6 6 | 7 7 7 7 7 | 8 8 8 8 8]
  * [1 0 0 0 1 | 0 0 4 5 0 | 1 5 1 0 1]
"""
x = np.array([
    # N=0
    [[[0, 1, 2],
      [1, 0, 4],
      [0, 2, 2],
      [1, 1, 2],
      [2, 1, 2]],
     [[1, 2, 1],
      [5, 2, 0],
      [4, 0, 5],
      [5, 2, 0],
      [0, 3, 1]],
     [[1, 2, 3],
      [5, 0, 0],
      [4, 2, 1],
      [5, 2, 0],
      [0, 3, 2]],
     [[1, 2, 0],
      [0, 4, 1],
      [5, 3, 0],
      [0, 0, 0],
      [3, 2, 4]],
     [[6, 7, 8],
      [6, 7, 8],
      [6, 7, 8],
      [6, 7, 8],
      [6, 7, 8]],
     [[1, 0, 1],
      [0, 0, 5],
      [0, 4, 1],
      [0, 5, 0],
      [1, 0, 1]]]
])


# 1,5,5,3 (NHWC)
"""
  *   C=0         C=1          C=2
  * [0 1 2 3 4 | 3 4 5 6 0 | 2 4 2 2 1]
  * [2 4 6 4 6 | 2 3 4 5 6 | 1 0 5 0 1]
  * [0 1 2 3 4 | 6 0 1 2 3 | 3 0 1 0 2]
  * [5 6 0 1 2 | 2 3 4 0 2 | 0 1 0 0 4]
  * [3 4 5 6 0 | 0 0 4 5 0 | 1 5 1 0 1]
"""

"""
  * ksize(2,3) strids(2,2) padding
  *   C=0         C=1          C=2
  * [0 0 1 2 3 4 0| 0 3 4 5 6 0 0 | 0 2 4 2 2 1 0]
  * [0 2 4 6 4 6 0| 0 2 3 4 5 6 0 | 0 1 0 5 0 1 0]
  * [0 0 1 2 3 4 0| 0 6 0 1 2 3 0 | 0 3 0 1 0 2 0]
  * [0 5 6 0 1 2 0| 0 2 3 4 0 2 0 | 0 0 1 0 0 4 0]
  * [0 3 4 5 6 0 0| 0 0 0 4 5 0 0 | 0 1 5 1 0 1 0]
  * [0 0 0 0 0 0 0| 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0]
"""

x = np.array([
    # N=0
    [[[0, 3, 2],
      [1, 4, 4],
      [2, 5, 2],
      [3, 6, 2],
      [4, 0, 2]],
     [[2, 2, 1],
      [4, 3, 0],
      [6, 4, 5],
      [4, 5, 0],
      [6, 6, 1]],
     [[0, 6, 3],
      [1, 0, 0],
      [2, 1, 1],
      [3, 2, 0],
      [4, 3, 2]],
     [[5, 2, 0],
      [6, 4, 1],
      [0, 3, 0],
      [1, 0, 0],
      [2, 2, 4]],
     [[3, 0, 1],
      [4, 0, 5],
      [5, 4, 1],
      [6, 5, 0],
      [0, 0, 1]]
    ]
])

'''expect SAME
[[[[4., 4., 4.],
  [6., 6., 5.],
  [6., 6., 2.]],

[[6., 6., 3.],
  [3., 3., 1.],
  [4., 3., 4.]],

[[4., 0., 5.],
  [6., 5., 1.],
  [0., 0., 1.]]]]
'''

print('x.shape', x.shape, "\n", x, "\n", x.flatten())
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)

x=tf.Variable(x)
out_valid1 = tf.nn.max_pool(x, ksize=[1,2,1,1], strides=[1, 2, 2, 1], padding='VALID')
out_same1 = tf.nn.max_pool(x, ksize=[1,1,2,1], strides=[1, 2, 2, 1], padding='SAME')
out_valid2 = tf.nn.max_pool(x, ksize=[1,2,2,1], strides=[1, 2, 2, 1], padding='VALID')
out_same2 = tf.nn.max_pool(x, ksize=[1,2,2,1], strides=[1, 2, 2, 1], padding='SAME')
out_valid3 = tf.nn.max_pool(x, ksize=[1,2,3,1], strides=[1, 2, 2, 1], padding='VALID')
out_same3 = tf.nn.max_pool(x, ksize=[1,2,3,1], strides=[1, 2, 2, 1], padding='SAME')
out_valid4 = tf.nn.max_pool(x, ksize=[1,3,3,1], strides=[1, 2, 2, 1], padding='VALID')
out_same4 = tf.nn.max_pool(x, ksize=[1,3,3,1], strides=[1, 2, 2, 1], padding='SAME')
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    sx = sess.run(x)

    print('out_valid1:', sess.run(out_valid1))
    print('out_same1:', sess.run(out_same1))
    print('out_valid2:', sess.run(out_valid2))
    print('out_same2:', sess.run(out_same2))
    print('out_valid3:', sess.run(out_valid3))
    print('out_same3:', sess.run(out_same3))
    print('out_valid4:', sess.run(out_valid4))
    print('out_same4:', sess.run(out_same4))
