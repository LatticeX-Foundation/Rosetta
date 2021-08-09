#!/bin/env python3

import latticex.rosetta as rtt
import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

protocol="SecureNN"

if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")

rtt.activate(protocol)

plain_a = np.array([[1.2345, -5.4321], [-2.3456, 6.5432]])
plain_b = np.array([[7.6543], [-0.1234]])
const_v = np.array([0.666])
print(plain_a)
print(plain_b)
print(const_v)

share_a = tf.Variable(rtt.private_input(0, plain_a))
share_b = tf.Variable(rtt.private_input(1, plain_b))
const_v = tf.constant(const_v)
print(share_a)
print(share_b)
print(const_v)

# init
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)

# ######### binary op
# Add
share_c = rtt.SecureAdd(share_a, share_b)
share_d = rtt.SecureAdd(share_c, share_a)
share_e = rtt.SecureAdd(share_d, share_b)
share_f = rtt.SecureAdd(share_e, const_v, rh_is_const=True)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureAdd:', sess.run(reveal0))

# Sub
share_c = rtt.SecureSub(share_a, share_b)
share_d = rtt.SecureSub(share_c, share_a)
share_e = rtt.SecureSub(share_d, share_b)
share_f = rtt.SecureSub(share_e, const_v, rh_is_const=True)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureSub:', sess.run(reveal0))

# Mul
share_c = rtt.SecureMul(share_a, share_b)
share_d = rtt.SecureMul(share_c, share_a)
share_e = rtt.SecureMul(share_d, share_b)
share_f = rtt.SecureMul(share_e, const_v, rh_is_const=True)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMul:', sess.run(reveal0))

# Truediv
share_c = rtt.SecureTruediv(share_a, share_b)
share_d = rtt.SecureTruediv(share_c, share_a)
share_e = rtt.SecureTruediv(share_d, share_b)
share_f = rtt.SecureTruediv(share_e, const_v, rh_is_const=True)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureTruediv:', sess.run(reveal0))

# Pow(const)
exponet = tf.Variable([1.1, 2.2], [3.3, 4.4])
share_f = rtt.SecurePow(share_a, exponet)  # the same as follow
#share_f = rtt.SecurePow(share_a, exponet, rh_is_const=True)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    print('SecurePow:', sess.run(reveal0))

# ######### compare op
# Greater
share_f = rtt.SecureGreater(share_a, share_b)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureGreater:', sess.run(reveal0))

# GreaterEqual
share_f = rtt.SecureGreaterEqual(share_a, share_b)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureGreaterEqual:', sess.run(reveal0))

# Less
share_f = rtt.SecureLess(share_a, share_b)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureLess:', sess.run(reveal0))

# LessEqual
share_f = rtt.SecureLessEqual(share_a, share_b)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureLessEqual:', sess.run(reveal0))


# ######### reduce op
# Max(None)
share_f = rtt.SecureMax(share_a, axis=None)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMax(axis=None):', sess.run(reveal0))

# Max(0)
share_f = rtt.SecureMax(share_a, axis=0)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMax(axis=0):', sess.run(reveal0))

# Max(1)
share_f = rtt.SecureMax(share_a, axis=1)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMax(axis=1):', sess.run(reveal0))

# Max([0,1])
share_f = rtt.SecureMax(share_a, axis=[0, 1])
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMax(axis=[0,1]):', sess.run(reveal0))


# Mean(None)
share_f = rtt.SecureMean(share_a, axis=None)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMean(axis=None):', sess.run(reveal0))

# Mean(0)
share_f = rtt.SecureMean(share_a, axis=0)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMean(axis=0):', sess.run(reveal0))

# Mean(1)
share_f = rtt.SecureMean(share_a, axis=1)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMean(axis=1):', sess.run(reveal0))

# Mean([0,1])
share_f = rtt.SecureMean(share_a, axis=[0, 1])
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMean(axis=[0,1]):', sess.run(reveal0))


# ######### log/log1p/sigmoid/matmul/
# Log
share_f = rtt.SecureLog(share_a)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureLog:', sess.run(reveal0))

# Log1p
share_f = rtt.SecureLog1p(share_a)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureLog1p:', sess.run(reveal0))

# Sigmoid
share_f = rtt.SecureSigmoid(share_a)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureSigmoid:', sess.run(reveal0))

# MatMul
share_e = rtt.SecureMatMul(share_a, share_a)
share_f = rtt.SecureMatMul(share_a, share_e)
reveal0 = rtt.SecureReveal(share_f)
with tf.Session() as sess:
    sess.run(init)
    print('SecureMatMul:', sess.run(reveal0))
rtt.deactivate()