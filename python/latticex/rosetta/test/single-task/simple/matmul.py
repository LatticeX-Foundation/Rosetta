#!/usr/bin/python

import latticex.rosetta as cb

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

protocol="helix"

if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")
cb.activate(protocol)

sess = None

xaa = np.array([[1.92, 0.2, 3], [-0.43, .0091, 1.3]])
xbb = np.array([[.2, 0.3], [-2, .3], [-1.111, -0.3]])
xa = tf.Variable(cb.private_input(0, xaa))
xb = tf.Variable(cb.private_input(1, xbb))

z1 = cb.SecureMatMul(xa, xb)
z0 = cb.SecureReveal(z1)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    print(sess.run(z1))
    print(sess.run(z0))


xaa = np.array([[1.0, 2.0], [3.0, 4.0]])
xbb = np.array([[1.0, 2.0], [3.0, 4.0]])
xa = tf.Variable(cb.private_input(0, xaa))
xb = tf.Variable(cb.private_input(1, xbb))

z1 = cb.SecureMatMul(xa, xb)
z0 = cb.SecureReveal(z1)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    print(sess.run(z1))
    print(sess.run(z0))

exit(0)


def test_matmul(xa, xb):
    global sess
    if sess is not None:
        sess.close()

    print("xa:\n", xa)
    print("xb:\n", xb)

    #
    init = tf.compat.v1.global_variables_initializer()
    sess = tf.compat.v1.Session()
    sess.run(init)

    print("=========================== tf op matmul 1")
    xc = tf.matmul(xa, xb)
    xcc = sess.run(xc)
    print("=========================== tf op matmul 2")
    print(xcc)

    ###########

    print("=========================== mpc op matmul 1")
    xc = cb.SecureMatMul(xa, xb)
    xcc = sess.run(xc)
    print("=========================== mpc op matmul 2")
    print(xcc)


# 2-d, (2,2) x (2,2)
xa = tf.Variable([[0, 0], [0, 0]])
xb = tf.Variable([[0, 0], [0, 0]])
test_matmul(xa, xb)

# 2-d, (2,2) x (2,2)
xa = tf.Variable([[1.0, 2.0], [3.0, 4.0]])
xb = tf.Variable([[5.0, 6.0], [7.0, 8.0]])
test_matmul(xa, xb)

# 2-d, (2,3) x (3,2)
xa = tf.Variable([[1.92, 0.2, 3], [-0.43, .0091, 1.3]])
xb = tf.Variable([[.2, 0.3], [-2, .3], [-1.111, -0.3]])
test_matmul(xa, xb)
