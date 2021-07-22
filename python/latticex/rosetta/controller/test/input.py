#!/usr/bin/env python3

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

protocol = "Helix"
rtt.activate(protocol)

input0 = rtt.private_input(0, 1.234)
input1 = rtt.private_input(1, 5.432)
input2 = rtt.private_input(2, 2.222)
print('input0:', input0)
print('input1:', input1)
print('input2:', input2)

i0 = tf.Variable(input0)
i1 = tf.Variable(input1)
i2 = tf.Variable(input2)

ii = rtt.SecureAdd(i0, i1)
ii = rtt.SecureAdd(ii, i2)
ir_add = rtt.SecureReveal(ii)  # i0 + i1 + i2

init = tf.global_variables_initializer()
with tf.Session() as sess1:
    sess1.run(init)
    print('rosetta add:', sess1.run(ir_add))


ii = rtt.SecureMul(i0, i1)
ii = rtt.SecureMul(ii, i2)
ir_mul = rtt.SecureReveal(ii)  # i0 * i1 * i2

init = tf.global_variables_initializer()
with tf.Session() as sess1:
    sess1.run(init)
    print('rosetta mul:', sess1.run(ir_mul))
