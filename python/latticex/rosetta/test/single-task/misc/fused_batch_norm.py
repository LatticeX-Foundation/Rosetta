#!/usr/bin/python

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf
xa = tf.Variable(
    [[[
        [5, 5],
        [7, 7],
        [9, 9],
        [11, 11],
        [13, 13],
        [15, 15]
    ]]], dtype=tf.float32
)

scale = tf.constant([4.0, 4.0])
offset = tf.constant([2.0, 2.0])

expected = np.array(
    [[[
        [-3.86, -3.86],
        [-1.51, -1.51],
        [0.83, 0.83],
        [3.17, 3.17],
        [5.51, 5.51],
        [7.86, 7.86]
    ]]], dtype=np.float64)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("input xa:", sess.run(xa))

###########
print("=========================== tf Train FusedBN 1")
xc, batch_mean, batch_var = tf.nn.fused_batch_norm(xa, scale, offset)
xcc = sess.run(xc)
print("=========================== tf Train FusedBN 2")
print("TF training res: ", xcc)

try:
    np.testing.assert_allclose(xcc, expected, rtol=0.01, atol=1.0/(2**8))
except Exception as e:
    print("FAIL !!!")
    print("context:", e)

Mean = tf.constant([10.0, 10.0], dtype=tf.float32)
Variance = tf.constant([11.67, 11.67])

print("=========================== tf Inference FusedBN 1")
infer_y, batch_mean, batch_var = tf.nn.fused_batch_norm(xa, scale, offset, Mean, Variance, is_training=False)

xcc = sess.run(infer_y)
print("=========================== tf Inference FusedBN 2")
print("TF inference res: ", xcc)


try:
    np.testing.assert_allclose(xcc, expected, rtol=0.01, atol=1.0/(2**8))
except Exception as e:
    print("FAIL !!!")
    print("context:", e)

import latticex.rosetta as rtt
# rtt.py_protocol_handler.set_loglevel(0)
# rtt.activate("Wolverine")

protocol="SecureNN"

if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")

rtt.activate(protocol)

xb = tf.Variable(
    [[[
        [5, 5],
        [7, 7],
        [9, 9],
        [11, 11],
        [13, 13],
        [15, 15]
    ]]], dtype=tf.float32
)

scale = tf.constant([4.0, 4.0])
offset = tf.constant([2.0, 2.0])

Mean = tf.constant([10.0, 10.0], dtype=tf.float32)
Variance = tf.constant([11.67, 11.67])

print("=========================== Rtt Train FusedBN 1")
#xc = cb.SecureAbs(xb)
# todo: there must have the mean and variance!
xc = rtt.SecureFusedBatchNorm(xb, scale, offset, Mean, Variance, is_training=False)
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

print("ZKP cipher :", sess.run(xc))

xcc = sess.run(rtt.SecureReveal(xc))
print("ZKP revealed : ", xcc)
xcc = xcc.astype(np.float64)
print("=========================== Rtt train FusedBN 2")

###########:

try:
    np.testing.assert_allclose(xcc, expected, rtol=0.01, atol=1.0/(2**8))
    print("SUCCESS!!!! PASSED!!!")
except Exception as e:
    print("FAIL!!!")
    print("context:", e)
