#!/usr/bin/python
#coding: utf-8
'''
function: test compare operator: >,<, >=,<=, ==
'''

import latticex.rosetta as rtt

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

from datetime import datetime

beginning = datetime.now()
# 6*5 matrix
xa = tf.Variable(
    [
        [1.892,		2, 		1.892, 	2, 		1.892, 	2, 		1.892, 	2],
        [-2.3, 		4.43, 	-2.3, 	4.43, 	-2.3, 	4.43, 	-2.3, 	4.43],
        [0.0091,    0.3,    0.0091, 0.3,    0.0091, 0.3,    0.0091, 0.3],
        [1.892,		2, 		1.892, 	2, 		1.892, 	2, 		1.892, 	2],
        [-2.3, 		4.43, 	-2.3, 	4.43, 	-2.3, 	4.43, 	-2.3, 	4.43],
        [0.0091, 	0.3, 	0.0091, 0.3, 	0.0091, 0.3, 	0.0091, 0.3]
    ], name='a'
)
xb = tf.Variable(
    [
        [2.892, 	2, 		2.892, 	2, 		2.892, 	2, 		2.892, 	2],
        [-2.3, 		4.43, 	-2.3, 	4.43, 	-2.3, 	4.43, 	-2.3, 	4.43],
        [0.0091, 	-0.3, 	0.0091, -0.3, 	0.0091, -0.3, 	0.0091, -0.3],
        [2.892, 	2, 		2.892, 	2, 		2.892, 	2, 		2.892, 	2],
        [-2.3, 		4.43, 	-2.3, 	4.43, 	-2.3, 	4.43, 	-2.3, 	4.43],
        [0.0091, 	-0.3, 	0.0091, -0.3, 	0.0091, -0.3, 	0.0091, -0.3]
    ], name='b'
)

#print("xa:\n", xa)
#print("xb:\n", xb)

protocol="SecureNN"
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")
rtt.activate(protocol)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

# add 10000 times of 6*6 matrix comparison
test_count = 1

def test_cmp(cmp_tensor, cmp_type, count=1000):
    elem_count = cmp_tensor.shape.num_elements()
    print("==> test tensor: {}, elems: {},  cmp type: {} ==".format(cmp_tensor, elem_count, cmp_type))

    start = datetime.now()
    for i in range(test_count):
        ret = sess.run(cmp_tensor)
        if i == count -1:
            print(ret)
    cost_time = datetime.now() - start
    print("==== cmp type: {}, cost: {}  {} s, total: {}  ====".format(cmp_type, cost_time, cost_time.seconds, datetime.now()-beginning))

if __name__ == "__main__":
    cmp_less = rtt.SecureLess(xa, xb)
    test_cmp(cmp_less, 'less', test_count)

    cmp_greater = rtt.SecureGreater(xa, xb)
    test_cmp(cmp_greater, 'greater', test_count)

    cmp_less_equal = rtt.SecureLessEqual(xa, xb)
    test_cmp(cmp_less_equal, 'less-equal', test_count)

    cmp_greater_equal = rtt.SecureGreaterEqual(xa, xb)
    test_cmp(cmp_greater_equal, 'cmp-greater-equal', test_count)

    add_op = rtt.SecureAdd(xa, xb)
    test_cmp(add_op, 'cmp-equal', test_count)

    sub_op = rtt.SecureSub(xa, xb)
    test_cmp(sub_op, 'cmp-equal', test_count)

    div_op = rtt.SecureDiv(xa, xb)
    test_cmp(div_op, 'cmp-equal', test_count)

    mul_op = rtt.SecureMul(xa, xb)
    test_cmp(mul_op, 'cmp-equal', test_count)

    cmp_equal = rtt.SecureEqual(xa, xb)
    test_cmp(cmp_equal, 'cmp-equal', test_count)

    rtt.deactivate()
    print('--------  ending ---------')