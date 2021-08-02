#!/usr/bin/python

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


xa = tf.Variable(
    [
        [1.892, 2],
        [3, 4.43],
        [.0091, .3]
    ]
)
# xb = tf.Variable(
#     [
#         [5.3,  .7],
#         [6, -0.7],
#         [3,  -2.001]
#     ]
# )

print("xa:\n", xa)
# print("xb:\n", xb)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("input xa:", sess.run(xa))

###########
print("=========================== tf op log1p 1")
xc = tf.log1p(xa)
xcc = sess.run(xc)
print("=========================== tf op log1p 2")
print("TF Log1p:", xcc)

import latticex.rosetta as cb 
#cb.py_protocol_handler.set_loglevel(0)
cb.activate("SecureNN")

xb = tf.Variable(
    [
        [1.892, 2],
        [3, 4.43],
        [.0091, .3]
    ]
)

init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
print("=========================== mpc op log1p 1")
xc = cb.SecureReveal(cb.SecureLog1p(xb))
xcc = sess.run(xc)
print("MPC revealed Log1p:", xcc)
print("=========================== mpc op log1p 2")
cb.deactivate()


###########
