#!/usr/bin/python
import os
print("my ProcessID:", os.getpid())
print("PWD:", os.getcwd())

#import unittest

import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf

# our package
import latticex.rosetta as cb

# two initial values with double type.

np_ori = np.array(
    [
        [10.1, 20.02],
        [30.003, 40.0004] 
    ],dtype = np.float64)

np_grad = np.array(
    [
        [1, 0.1],
        [0.01, 0.001]
    ], dtype = np.float64)

# the precision requirement of saving and restoring a float number
PRECISION = 1.0/1000

input_val = tf.Variable(np_ori, dtype = tf.double)
input_alpha = tf.constant(0.1, dtype = tf.double)
input_grad = tf.constant(np_grad, dtype = tf.double)

init_op = tf.compat.v1.global_variables_initializer()

#### To run this native showcase, you must expose the 
#    tf.training_ops.apply_gradient_descent function
#    first, becasue the installed tendorflow does not
#    expose this function by default.
# print("========TF native apply_gradient_descent showcase================")
# with tf.compat.v1.Session() as native_show_sess:
#     native_show_sess.run(init_op)
#     apply_sgd = tf.training_ops.apply_gradient_descent(input_val,
#                                                     input_alpha,
#                                                     input_grad)
    
#     print("original input variable:\n", native_show_sess.run(input_val))
#     print("output:\n", native_show_sess.run(apply_sgd))
#     print("updated input variable:\n", native_show_sess.run(input_val))

print("========MPC apply_gradient_descent showcase================")
with tf.compat.v1.Session() as MPC_show_sess:
    MPC_show_sess.run(init_op)
    apply_sgd = cb.SecureApplyGradientDescent(input_val,
                                        input_alpha,
                                        input_grad,
                                        use_locking=False)
    
    print("original input variable:\n", MPC_show_sess.run(input_val))
    print("output:\n", MPC_show_sess.run(apply_sgd))
    print("updated input variable:\n", MPC_show_sess.run(input_val))

print("========Case Two: static replacement======================")
with tf.compat.v1.Session() as MPC_show_sess:
    MPC_show_sess.run(init_op)
    apply_sgd = tf.training_ops.apply_gradient_descent(input_val,
                                        input_alpha,
                                        input_grad,
                                        use_locking=False)
    
    print("original input variable:\n", MPC_show_sess.run(input_val))
    print("output:\n", MPC_show_sess.run(apply_sgd))
    print("updated input variable:\n", MPC_show_sess.run(input_val))

print("END")