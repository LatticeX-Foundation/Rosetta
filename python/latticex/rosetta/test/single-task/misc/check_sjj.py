#!/usr/bin/env python3

import latticex.rosetta as rtt
import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

import logging
logging.basicConfig(level=logging.DEBUG)
rtt.set_backend_loglevel(2)

protocol="SecureNN"
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")
rtt.activate(protocol)

rtt.activate(protocol)
print("rtt.get_protocol_name():", rtt.get_protocol_name())

patyid = rtt.get_party_id()

# float * tf.Variable()
num_a = np.array(
    [
        [2000.0, 20000.0],
        [20000 * 100.0, 200.0]
    ], dtype =np.float_)
num_b = np.array(
    [
        [50000, -50000.0],
        [-50000, -0.02] 
    ],dtype= np.float_)

X = tf.Variable(rtt.private_input(0, num_a))
Y = tf.Variable(rtt.private_input(1, num_b))
CX = tf.constant(num_a)
CY = tf.constant(num_b)

expected = num_a / num_b

floor_expected = np.floor_divide(num_a, num_b)


a1 =  X / Y
floor1 = tf.floordiv(X, Y)

a2 = X / CY
floor2 = tf.floordiv(X, CY)

# in this case, it is hard to get the correct result from big shared right!
a3 = CX / Y
floor3 = tf.floordiv(CX, Y)

a4 = CX / CY
floor4 = tf.floordiv(CX, CY)

r_a1 = rtt.SecureReveal([a1, floor1])
r_a2 = rtt.SecureReveal([a2, floor2])
r_a3 = rtt.SecureReveal([a3, floor3])
r_a4 = rtt.SecureReveal([a4, floor4])

init = tf.global_variables_initializer()
with tf.Session() as sess1:
    sess1.run(init)
    rr_a1 = sess1.run(r_a1)
    rr_a2 = sess1.run(r_a2)
    rr_a3 = sess1.run(r_a3)
    rr_a4 = sess1.run(r_a4)

    print("expected:", expected)
    print("floor expected:", floor_expected)

    print("**** Div [S vs S] ****")
    print('rr_a1:', rr_a1)
    print("**** Div [S vs C] ****")
    print('rr_a2:', rr_a2)
    print("**** Div [C vs S] ****")
    print('rr_a3:', rr_a3)

    print("**** Div [C vs C] ****")
    print('rr_a4:', rr_a4)
rtt.deactivate()