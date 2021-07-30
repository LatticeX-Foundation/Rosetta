#!/usr/bin/python

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

patyid = rtt.get_party_id()


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
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)
s = ''
for i in k.flatten():
    s += str(i)
    s += ','
print('k:', s)


x = tf.Variable(rtt.private_input(0, x))
kernel = tf.Variable(rtt.private_input(0, k))
out_mat = tf.matmul(x, kernel)
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    print('rout_mat:', sess.run(rtt.SecureReveal(out_mat)))


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
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)
s = ''
for i in k.flatten():
    s += str(i)
    s += ','
print('k:', s)


x = tf.Variable(rtt.private_input(0, x))
kernel = tf.Variable(rtt.private_input(0, k))
out_valid = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='SAME')
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    sx = sess.run(x)
    sk = sess.run(kernel)
    print('sx', sx)
    print('sk', sk)

    #print('rx', sess.run(rtt.SecureReveal(sx)))
    #print('rk', sess.run(rtt.SecureReveal(sk)))

    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))

    print('rout_valid:', sess.run(rtt.SecureReveal(out_valid)))
    print('rout_same:', sess.run(rtt.SecureReveal(out_same)))

#
#
#
#
#
#


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
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)
s = ''
for i in k.flatten():
    s += str(i)
    s += ','
print('k:', s)


x = tf.Variable(rtt.private_input(0, x))
kernel = tf.Variable(rtt.private_input(0, k))
out_valid = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='SAME')
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    sx = sess.run(x)
    sk = sess.run(kernel)
    print('sx', sx)
    print('sk', sk)

    #print('rx', sess.run(rtt.SecureReveal(sx)))
    #print('rk', sess.run(rtt.SecureReveal(sk)))

    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))

    print('rout_valid:', sess.run(rtt.SecureReveal(out_valid)))
    print('rout_same:', sess.run(rtt.SecureReveal(out_same)))

#
#
#
#
#
#


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
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)
s = ''
for i in k.flatten():
    s += str(i)
    s += ','
print('k:', s)


x = tf.Variable(rtt.private_input(0, x))
kernel = tf.Variable(rtt.private_input(0, k))
out_valid = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='SAME')
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    sx = sess.run(x)
    sk = sess.run(kernel)
    print('sx', sx)
    print('sk', sk)

    #print('rx', sess.run(rtt.SecureReveal(sx)))
    #print('rk', sess.run(rtt.SecureReveal(sk)))

    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))

    print('rout_valid:', sess.run(rtt.SecureReveal(out_valid)))
    print('rout_same:', sess.run(rtt.SecureReveal(out_same)))

#
#
# FFFFFFFFFFFFloat
#
#
#


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
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)
s = ''
for i in k.flatten():
    s += str(i)
    s += ','
print('k:', s)


x = tf.Variable(rtt.private_input(0, x))
kernel = tf.Variable(rtt.private_input(0, k))
out_valid = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='SAME')
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    sx = sess.run(x)
    sk = sess.run(kernel)
    print('sx', sx)
    print('sk', sk)

    #print('rx', sess.run(rtt.SecureReveal(sx)))
    #print('rk', sess.run(rtt.SecureReveal(sk)))

    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))

    print('rout_valid:', sess.run(rtt.SecureReveal(out_valid)))
    print('rout_same:', sess.run(rtt.SecureReveal(out_same)))


#
#
# FFFFFFFFFFFFloat  Neg
#
#
#


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
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)
s = ''
for i in k.flatten():
    s += str(i)
    s += ','
print('k:', s)


x = tf.Variable(rtt.private_input(0, x))
kernel = tf.Variable(rtt.private_input(0, k))
out_valid = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='VALID')
out_same = tf.nn.conv2d(x, kernel, strides=[1, 1, 1, 1], padding='SAME')
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    sx = sess.run(x)
    sk = sess.run(kernel)
    print('sx', sx)
    print('sk', sk)

    #print('rx', sess.run(rtt.SecureReveal(sx)))
    #print('rk', sess.run(rtt.SecureReveal(sk)))

    print(out_valid)
    print(out_same)
    print("out_valid:", sess.run(out_valid))
    print("out_same:", sess.run(out_same))

    print('rout_valid:', sess.run(rtt.SecureReveal(out_valid)))
    print('rout_same:', sess.run(rtt.SecureReveal(out_same)))


rtt.deactivate()

exit(0)
