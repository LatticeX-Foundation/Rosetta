#!/usr/bin/python

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

tf.set_random_seed(0)
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

print("\n\n\n")
print("conv2dshape: 1 3 3 1 * 2 2 1 1 -> 1 2 2 1/1 3 3 1")

print("=========================== simple ....")
# 1,3,3,1
"""
0 1 0
3 4 1
2 5 1
paddings:
0 1 0 0
3 4 1 0
2 5 1 0
0 0 0 0
"""
x = np.array([[[[0],
                [3],
                [2]],
               [[1],
                [4],
                [5]],
               [[0],
                [1],
                [1]]]])
# 2,2,1,1
"""
1 3
2 4
"""
k = np.array([
    [[[1]],
     [[2]]],
    [[[3]],
     [[4]]]
])
print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

# 1,2,2,1
"""
25 13
39 21
"""
out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
# 1,3,3,1
"""
25 13  2
39 21  3
17  8  1
"""
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))
print("=========================== simple ....")

print("\n\n\n")
print("conv2dshape: 1 3 3 1 * 2 2 1 2 -> 1 2 2 2/1 3 3 2")

print("=========================== simple ....")
# 1,3,3,1
"""
0   1 -1
-1  5  0
2  -3  1
paddings:
0   1 -1  0
-1  5  0  0
2  -3  1  0
0   0  0  0
"""
x = np.array([[[[0],
                [-1],
                [2]],
               [[1],
                [5],
                [-3]],
               [[-1],
                [0],
                [1]]]])
# 2,2,1,2
"""
1 3 | 0 -1
2 4 | 4  2
"""
k = np.array(
    [[[[1, 0]],
      [[2, 4]]],
        [[[3, -1]],
         [[4, 2]]]]
)
print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

# 1,2,2,2
"""
21  8 |  5  21
 6  3 | -3 -10
"""
out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
# 1,3,3,2
"""
21  8 -1 |  5  21  0
 6  3  2 | -3 -10  4
-7  0  1 |  3  -1  0
"""
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))
print("=========================== simple ....")

#
##
#
##
#
print("\n\n\n")
print("conv2dshape: 2 3 3 1 * 2 2 1 1 -> 2 2 2 1/2 3 3 1")

print("=========================== simple ....")
# 2,3,3,1 (NHWC)
"""
    |   C=0    
---------------
N=0 | 0   1 -1 
N=0 | -1  5  0 
N=0 | 2  -3  1 
---------------
N=1 | 0   1 -1 
N=1 | -1  2  0 
N=1 | 2  -3  1 
---------------
padding:

"""
x = np.array([
    # N=0
    [[[0],
      [-1],
      [2]],
     [[1],
      [5],
      [-3]],
     [[-1],
      [0],
      [1]]],

    # N=1
    [[[0],
      [-1],
      [2]],
     [[1],
      [2],
      [-3]],
     [[-1],
      [0],
      [1]]]
])

# 2,2,1,1
"""
     |  OC=0 
-------------
IC=0 | 1  -1 
IC=0 | 0   2 
-------------
"""
k = np.array([
    [[[1]],
     [[0]]],
    [[[-1]],
     [[2]]]
])

print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

# 2,2,2,1
out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
"""
    |  OC=0 
------------
N=0 |   9  2
N=0 | -12  7 
------------
N=1 |   3  2 
N=1 |  -9  4
------------
"""
# 2,3,3,1
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
"""
    |   OC=0   
---------------
N=0 |  9  2 -1 
N=0 |-12  7  0 
N=0 |  5 -4  1 
---------------
N=1 |  3  2 -1 
N=1 | -9  4  0 
N=1 |  5 -4  1 
---------------
"""
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:\n", sess.run(out_valid))
    print("out_same:\n", sess.run(out_same))
print("=========================== simple ....")


print("\n\n\n")
print("conv2dshape: 2 3 3 1 * 2 2 1 2 -> 2 2 2 2/2 3 3 2")

print("=========================== simple ....")
# 2,3,3,1 (NHWC)
"""
    |   C=0    
---------------
N=0 | 0   1 -1 
N=0 | -1  5  0 
N=0 | 2  -3  1 
---------------
N=1 | 0   1 -1 
N=1 | -1  2  0 
N=1 | 2  -3  1 
---------------
padding:

"""
x = np.array([
    # N=0
    [[[0],
      [-1],
      [2]],
     [[1],
      [5],
      [-3]],
     [[-1],
      [0],
      [1]]],

    # N=1
    [[[0],
      [-1],
      [2]],
     [[1],
      [2],
      [-3]],
     [[-1],
      [0],
      [1]]]
])

# 2,2,1,2
"""
     |  OC=0 |  OC=1 
---------------------
IC=0 | 1  -1 | 1   0
IC=0 | 0   2 | 2  -1 
---------------------
"""
k = np.array([
    [[[1, 1]],
     [[0, 2]]],
    [[[-1, 0]],
     [[2, -1]]]
])

print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

# 2,2,2,1
out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
"""
    |  OC=0  |  OC=1 
-------------|------
N=0 |   9  2 |  -7 11
N=0 | -12  7 |   6 -2 
-------------|------
N=1 |   3  2 |  -4  5
N=1 |  -9  4 |   6 -5
-------------|------
"""
# 2,3,3,1
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
"""
    |   OC=0   |   OC=1   
--------------------------
N=0 |  9  2 -1 | -7  11 -1 
N=0 |-12  7  0 |  6  -2  2 
N=0 |  5 -4  1 |  2  -3  1 
--------------------------
N=1 |  3  2 -1 | -4  5  -1 
N=1 | -9  4  0 |  6 -5   2 
N=1 |  5 -4  1 |  2 -3   1 
--------------------------
"""
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:\n", sess.run(out_valid))
    print("out_same:\n", sess.run(out_same))
print("=========================== simple ....")


print("\n\n\n")
print("=========================== simple ....")


x = np.array([
    [0,2],
    [1,4]
])

k = np.array([
    [1, 2],
    [0, 1]
])

x = np.array([
    [0, 1, 2],
    [1, 0, 4],
    [0, 2, 2],
    [1, 1, 2]
])

k = np.array([
    [0, 1, 2, 2],
    [1, 0, 1, 4],
    [2, 1, 0, 2]
])


print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)
out_mat = tf.matmul(xx, kk)
with tf.Session() as sess:
    print(out_mat)
    print("out_mat:\n", sess.run(out_mat))
print("=========================== simple ....")


print("\n\n\n\n\n\n")
print("conv2dshape: 1 5 5 3 * 3 3 3 4 -> 1 3 3 4/1 5 5 4")

print("=========================== simple ....")
"""
  C=0         C=1          C=2
-----------------------------------
[0 1 1 1 1 | 1 2 2 2 0 | 2 1 3 0 1]
[1 5 5 0 0 | 0 2 0 4 0 | 4 0 0 1 5]
[0 4 4 5 0 | 2 0 2 3 4 | 2 5 1 0 1]
[1 5 5 0 0 | 1 2 2 0 5 | 2 0 0 0 0]
[2 0 0 3 1 | 1 3 3 2 0 | 2 1 2 4 1]
-----------------------------------
"""
# 1,5,5,3 (NHWC)
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
     [[1, 0, 1],
      [0, 0, 5],
      [0, 4, 1],
      [0, 5, 0],
      [1, 0, 1]]]
])


"""
       OC=0   OC=1   OC=2  OC=3
---------------------------------
IC=0 [0 1 1][1 2 2][2 0 0][2 0 0]
IC=0 [1 1 1][2 1 2][0 1 1][1 1 0]
IC=0 [1 1 0][2 0 0][3 1 0][3 1 1]
---------------------------------
IC=1 [1 0 0][0 0 4][1 1 1][4 1 1]
IC=1 [0 0 1][0 2 2][1 0 0][0 0 0]
IC=1 [0 0 0][0 1 1][1 1 1][0 0 1]
---------------------------------
IC=2 [2 3 0][1 2 3][0 4 0][2 3 0]
IC=2 [0 3 0][3 0 3][1 3 1][1 4 3]
IC=2 [0 1 1][3 0 0][2 2 1][2 1 0]
---------------------------------
"""
# 3,3,3,4
k = np.array([
    [[[0, 1, 2, 2],
      [1, 0, 1, 4],
      [2, 1, 0, 2]],
     [[1, 2, 0, 1],
      [0, 0, 1, 0],
      [0, 3, 1, 1]],
     [[1, 2, 3, 3],
      [0, 0, 1, 0],
      [0, 3, 2, 2]]],

    [[[1, 2, 0, 0],
      [0, 0, 1, 1],
      [3, 2, 4, 3]],
     [[1, 1, 1, 1],
      [0, 2, 0, 0],
      [3, 0, 3, 4]],
     [[1, 0, 1, 1],
      [0, 1, 1, 0],
      [1, 0, 2, 1]]],

    [[[1, 2, 0, 0],
      [0, 4, 1, 1],
      [0, 3, 0, 0]],
     [[1, 2, 1, 0],
      [1, 2, 0, 0],
      [0, 3, 1, 3]],
     [[0, 0, 0, 1],
      [0, 1, 1, 1],
      [1, 0, 1, 0]]]
])

print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

# 1 3 3 4
out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')

# 1 5 5 4
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:\n", sess.run(out_valid))
    print("out_same:\n", sess.run(out_same))
print("=========================== simple ....")


print("\n\n\n")
print("=========================== simple ....")

# 1,6,5,3 (NHWC)
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

# 3,2,3,4
k = np.array([
    [[[0, 1, 2, 2],
      [1, 0, 1, 4],
      [2, 1, 0, 2]],
     [[1, 2, 3, 3],
      [0, 0, 1, 0],
      [0, 3, 2, 2]]],

    [[[1, 2, 0, 0],
      [0, 0, 1, 1],
      [3, 2, 4, 3]],
     [[1, 0, 1, 1],
      [0, 1, 1, 0],
      [1, 0, 2, 1]]],

    [[[1, 2, 0, 0],
      [0, 4, 1, 1],
      [0, 3, 0, 0]],
     [[0, 0, 0, 1],
      [0, 1, 1, 1],
      [1, 0, 1, 0]]]
])


print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))
print("=========================== simple ....")


print("\n\n\n")
print("=========================== simple ....")

# 3,2,4,3 (NHWC)
x = np.array([
    # N=0
    [[[0, 1, 2],
      [1, 0, 4],
      [0, 2, 2],
      [2, 1, 2]],
     [[1, 2, 3],
      [5, 0, 0],
      [5, 2, 0],
      [0, 3, 2]]],

    # N=1
    [[[1, 2, 0],
      [0, 4, 1],
      [0, 0, 0],
      [3, 2, 4]],
     [[1, 0, 1],
      [0, 0, 5],
      [0, 5, 0],
      [1, 0, 1]]],

    # N=2
    [[[1, 2, 1],
      [5, 2, 0],
      [5, 2, 0],
      [0, 3, 1]],
     [[6, 7, 8],
      [6, 7, 8],
      [6, 7, 8],
      [6, 7, 8]]],
])

# 2,2,3,4
k = np.array([
    [[[0, 1, 2, 2],
      [1, 0, 1, 4],
      [2, 1, 0, 2]],
     [[1, 2, 3, 3],
      [0, 0, 1, 0],
      [0, 3, 2, 2]]],

    [[[1, 2, 0, 0],
      [0, 4, 1, 1],
      [0, 3, 0, 0]],
     [[0, 0, 0, 1],
      [0, 1, 1, 1],
      [1, 0, 1, 0]]]
])


print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))
print("=========================== simple ....")


# FFFFFFFFFFFFFFFFFloat
print("\n\n\n")
print("=========================== simple ....")

# 3,2,4,3 (NHWC)
x = np.array([
    # N=0
    [[[0.3, 1, 2],
      [1, 0, 4.2],
      [0, 2, 2],
      [2, 1.23, 2]],
     [[1, 2.23, 3],
      [5, 0.23, 0],
      [5, 2.23, 0],
      [0, 3.23, 2]]],

    # N=1
    [[[1, 2.8, 0],
      [0, 4.8, 1],
      [0, 0.8, 0],
      [3, 2.8, 4]],
     [[1, 0.8, 1],
      [0, 0.8, 5],
      [0, 5.8, 0],
      [1, 0.8, 1]]],

    # N=2
    [[[1, 2, 0.1],
      [5, 2, 0.0],
      [5, 2, 0.0],
      [0, 3, 0.1]],
     [[6, 7, 0.8],
      [6, 7, 0.8],
      [6, 7, 0.8],
      [6, 7, 0.8]]],
])

# 2,2,3,4
k = np.array([
    [[[0, 1.01, 2, 2.6],
      [1, 0.01, 1, 4.6],
      [2, 1.01, 0, 2.6]],
     [[1, 2.01, 3, 3.6],
      [0, 0.01, 1, 0.6],
      [0, 3.01, 2, 2.6]]],

    [[[1, 2.4, 0, 0],
      [0, 4.4, 1, 1],
      [0, 3.4, 0, 0]],
     [[0, 0.4, 0, 1],
      [0, 1.4, 1, 1],
      [1, 0.4, 1, 0]]]
])


print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))
print("=========================== simple ....")



# FFFFFFFFFFFFloat  Neg
print("\n\n\n")
print("=========================== simple ....")

# 3,2,4,3 (NHWC)
x = np.array([
    # N=0
    [[[-0.3, 1, 2],
      [-1, 0, 4.2],
      [-0, 2, 2],
      [-2, 1.23, 2]],
     [[-1, 2.23, 3],
      [-5, 0.23, 0],
      [-5, 2.23, 0],
      [-0, 3.23, 2]]],

    # N=1
    [[[1, 2.8, -0],
      [0, 4.8, -1],
      [0, 0.8, -0],
      [3, 2.8, -4]],
     [[1, 0.8, -1],
      [0, 0.8, -5],
      [0, 5.8, -0],
      [1, 0.8, -1]]],

    # N=2
    [[[1, 2, 0.1],
      [5, 2, 0.0],
      [5, 2, 0.0],
      [0, 3, 0.1]],
     [[6, 7, 0.8],
      [6, 7, 0.8],
      [6, 7, 0.8],
      [6, 7, 0.8]]],
])

# 2,2,3,4
k = np.array([
    [[[0, 1.01, 2, 2.6],
      [1, 0.01, 1, 4.6],
      [2, 1.01, 0, 2.6]],
     [[1, 2.01, 3, 3.6],
      [0, 0.01, 1, 0.6],
      [0, 3.01, 2, 2.6]]],

    [[[1, -2.4, 0, 0],
      [0, -4.4, 1, 1],
      [0, -3.4, 0, 0]],
     [[0, -0.4, 0, 1],
      [0, -1.4, 1, 1],
      [1, -0.4, 1, 0]]]
])


print('x.shape', x.shape, "\n", x, "\n", x.flatten())
print('k.shape', k.shape, "\n", k, "\n", k.flatten())

# #####################
xx = tf.constant(x, dtype=tf.float32)
kk = tf.constant(k, dtype=tf.float32)

out_valid = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(xx, kk, strides=[1, 1, 1, 1], padding='SAME')
with tf.Session() as sess:
    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))
print("=========================== simple ....")
