#!/usr/bin/python

import latticex.rosetta as rtt

import tensorflow as tf
import sys, time
import numpy as np
np.set_printoptions(suppress=True)

rtt.activate("SecureNN")

# xa = tf.Variable(rtt.private_input(0,
#     [
#         [0.12345678, 0.234567890],
#         [0.34567890, 0.03],
#         [1.3, 1.89],
#         [-1.3, -1.89],
#         [2.2, -2.2],
#         [-4.0, 4.0]
#     ]
# ))

xa = tf.constant(np.full(102400, 2.2))
wa = tf.Variable(rtt.private_input(0, np.full(102400, 2.2)))

ret = wa # tf.multiply(xa, wa)

print("xa:\n", xa)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

# print("X:\n", sess.run(xa * 2))
###########
print("=========================== tf op sigmoid 1")
xc = tf.sigmoid(ret)
# xc = tf.nn.sigmoid(ret)
xcc = sess.run(xc)
print("=========================== tf op sigmoid 2")
print(xcc)

# print("=========================== mpc op sigmoid 1")
start = time.time()
xc = rtt.SecureReveal(tf.sigmoid(ret))
xcc = sess.run(xc)
print("secure_sigmoid cost: ", time.time()-start)
# print("=========================== mpc op sigmoid 2")
print(xcc)

# print("=========================== mpc op sigmoidV2 1")
start = time.time()
xc = rtt.SecureReveal(rtt.SecureSigmoidV2(ret))
xcc = sess.run(xc)
print("secure_sigmoid_6Pieces_Python cost: ", time.time()-start)
# print("=========================== mpc op sigmoid 2")
print(xcc)

# print("=========================== mpc op sigmoidV2 1")
start = time.time()
xc = rtt.SecureReveal(rtt.SecureSigmoidV3(ret))
xcc = sess.run(xc)
print("secure_sigmoid_3Pieces_Python cost: ", time.time()-start)
# print("=========================== mpc op sigmoid 2")
print(xcc)

# print("=========================== mpc op sigmoidV2 1")
start = time.time()
xc = rtt.SecureReveal(rtt.SecureSigmoidChebyshev(ret))
xcc = sess.run(xc)
print("secure_Sigmoid_Chebyshev_Python cost: ", time.time()-start)
print("=========================== mpc op sigmoid 2")
print(xcc)

###########
