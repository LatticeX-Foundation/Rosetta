#!/usr/bin/python
import os
print("my ProcessID:", os.getpid())
print("PWD:", os.getcwd())

#import unittest

import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf

# our package
import latticex.rosetta as rtt

protocol="SecureNN"
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")
rtt.activate(protocol)

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

from tensorflow.python.framework import constant_op
alpha = constant_op.constant(0.1, dtype = tf.double)

input_val = tf.Variable(np_ori, dtype = tf.double)
input_grad = tf.Variable(np_grad, dtype = tf.double)

init_op = tf.compat.v1.global_variables_initializer()

from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
def get_rtt_ref_var(refVar):
        """get rtt variable from rtt tensor"""
        rtt_var_op_def_name  = ("VariableV2", )
        
        if (isinstance(refVar, rtt_ts.RttTensor)):
            dest_tensor = refVar._raw
            while (dest_tensor.op.op_def.name not in rtt_var_op_def_name):
                # assert len(dest_tensor.op.inputs) > 0, "input parameters 'ref' is incorrect!"
                dest_tensor = dest_tensor.op.inputs[0]
            return dest_tensor
        else:
            return refVar

print("========MPC apply_gradient_descent showcase================")
with tf.compat.v1.Session() as MPC_show_sess:
    MPC_show_sess.run(init_op)
    apply_sgd = rtt.SecureApplyGradientDescent(get_rtt_ref_var(input_val),
                                        alpha,
                                        input_grad,
                                        use_locking=False)
    
    print("original input variable:\n", MPC_show_sess.run(input_val))
    print("output:\n", MPC_show_sess.run(rtt.SecureReveal(apply_sgd)))
    print("updated input variable:\n", MPC_show_sess.run(rtt.SecureReveal(input_val)))
rtt.deactivate()