#!/usr/bin/python

import latticex.rosetta as rtt
import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

tf.set_random_seed(0)

# rtt.set_backend_loglevel(2)

protocol="SecureNN"
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")

rtt.activate(protocol)

print("rtt.get_protocol_name():", rtt.get_protocol_name())

patyid = rtt.get_party_id("")

# 1,5,5,3 (NHWC)
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

'''expects:
out_valid1: 
[[[[2 3 2]
   [6 5 5]
   [6 6 2]]

  [[5 6 3]
   [2 3 1]
   [4 3 4]]]]

out_same1:
[[[[1 4 4]
   [3 6 2]
   [4 0 2]]

  [[1 6 3]
   [3 2 1]
   [4 3 2]]

  [[4 0 5]
   [6 5 1]
   [0 0 1]]]]

out_valid2: 
[[[[4 4 4]
   [6 6 5]]

  [[6 6 3]
   [3 3 1]]]]

out_same2: 
[[[[4 4 4]
   [6 6 5]
   [6 6 2]]

  [[6 6 3]
   [3 3 1]
   [4 3 4]]

  [[4 0 5]
   [6 5 1]
   [0 0 1]]]]

out_valid3: 
[[[[6 5 5]
   [6 6 5]]

  [[6 6 3]
   [4 3 4]]]]

out_same3: 
[[[[4 4 4]
   [6 6 5]
   [6 6 2]]

  [[6 6 3]
   [6 4 1]
   [4 3 4]]

  [[4 0 5]
   [6 5 5]
   [6 5 1]]]]

out_valid4: 
[[[[6 6 5]
   [6 6 5]]

  [[6 6 5]
   [6 5 4]]]]

out_same4: 
[[[[4 4 4]
   [6 6 5]
   [6 6 2]]

  [[6 6 3]
   [6 5 5]
   [6 6 4]]

  [[6 4 5]
   [6 5 5]
   [6 5 4]]]]
'''

print('x.shape', x.shape, "\n", x, "\n", x.flatten())
s = ''
for i in x.flatten():
    s += str(i)
    s += ','
print('x:', s)

x = tf.Variable(rtt.private_input(0, x))
# kernel = tf.Variable(rtt.private_input(0, y))
out_valid1 = tf.nn.max_pool(x, ksize=[1,2,1,1], strides=[1, 2, 2, 1], padding='VALID')
out_same1 = tf.nn.max_pool(x, ksize=[1,1,2,1], strides=[1, 2, 2, 1], padding='SAME')

out_valid2 = tf.nn.max_pool(x, ksize=[1,2,2,1], strides=[1, 2, 2, 1], padding='VALID')
out_same2 = tf.nn.max_pool(x, ksize=[1,2,2,1], strides=[1, 2, 2, 1], padding='SAME')

out_valid3 = tf.nn.max_pool(x, ksize=[1,2,3,1], strides=[1, 2, 2, 1], padding='VALID')
out_same3 = tf.nn.max_pool(x, ksize=[1,2,3,1], strides=[1, 2, 2, 1], padding='SAME')

out_valid4 = tf.nn.max_pool(x, ksize=[1,3,3,1], strides=[1, 2, 2, 1], padding='VALID')
out_same4 = tf.nn.max_pool(x, ksize=[1,3,3,1], strides=[1, 2, 2, 1], padding='SAME')
# out_valid4 = tf.nn.max_pool(x, ksize=[1,2,3,1], strides=[1, 2, 2, 1], padding='VALID')
init = tf.compat.v1.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    sx = sess.run(x)

    print('out_valid1:', sess.run(rtt.SecureReveal(out_valid1))) # ok
    print('out_same1:', sess.run(rtt.SecureReveal(out_same1))) # ok
    print('out_valid2:', sess.run(rtt.SecureReveal(out_valid2))) # ok
    print('out_same2:', sess.run(rtt.SecureReveal(out_same2))) # ok
    print('out_valid3:', sess.run(rtt.SecureReveal(out_valid3))) # ok
    print('out_same3:', sess.run(rtt.SecureReveal(out_same3))) # ok
    print('out_valid4:', sess.run(rtt.SecureReveal(out_valid4))) # ok
    print('out_same4:', sess.run(rtt.SecureReveal(out_same4))) # ok

    print("end.")
