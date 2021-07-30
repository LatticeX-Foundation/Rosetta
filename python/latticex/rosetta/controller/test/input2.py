#!/usr/bin/env python3

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

protocol = "Helix"
rtt.activate(protocol)

x = np.array([[2.0, 2.1], [2.2, 2.3]])
y = np.array([[1, 2.0], [1, 3.0]])
z = np.array([[[0, 1, 2.4, 0, 1, 2.9]],
              [[0.3, 1, 2, 0, 1.2, 2]]])

input0 = rtt.private_input(0, x)
input1 = rtt.private_input(1, y)
input2 = rtt.private_input(2, z)
print('input0:', type(input0), input0)
print('input1:', type(input1), input1)
print('input2:', type(input2), input2)

i0 = tf.Variable(input0)
i1 = tf.Variable(input1)
i2 = tf.Variable(input2)

ii = rtt.SecureMatMul(i0, i1)
ir_matmul = rtt.SecureReveal(ii)

init = tf.global_variables_initializer()
with tf.Session() as sess1:
    sess1.run(init)
    print('rosetta matmul:', sess1.run(ir_matmul))
