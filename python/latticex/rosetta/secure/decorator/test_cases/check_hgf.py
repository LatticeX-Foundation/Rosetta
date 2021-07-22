#!/usr/bin/env python3
# test cases for hgf
# test cases are mostly for zk-Wolverie

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

import logging
logging.basicConfig(level=logging.DEBUG)
rtt.set_backend_loglevel(2)

protocol = "Wolverine"
rtt.activate(protocol)

patyid = rtt.get_party_id()
print("rtt.get_protocol_name: ", rtt.get_protocol_name(), "party: ", )


# float * tf.Variable()
num_a = np.ones([10,10])
num_b = np.ones([10,10]) * 2
num_neg_a = -num_a


# only ALICE could own private data
X = tf.Variable(rtt.private_input(0, num_a))
Y = tf.Variable(rtt.private_input(0, num_b))
P = tf.Variable(rtt.private_input(0, num_neg_a))
CX = tf.constant(num_a)
CY = tf.constant(num_b)

# add
Add = X + Y
# sub
Sub = Y - X
# mul
Mul = X * Y
# matmul
Matmul = tf.matmul(X, Y)
# bias_add
bias_y = tf.Variable(rtt.private_input(0, np.ones(10)))
BiasAdd = tf.nn.bias_add(X, bias_y)
# relu
Relu1 = tf.nn.relu(X) # 1,1,1
Relu0 = tf.nn.relu(P)

r_add = rtt.SecureReveal(Add)
r_sub = rtt.SecureReveal(Sub)
r_mul = rtt.SecureReveal(Mul)
r_matmul = rtt.SecureReveal(Matmul)
r_bias_add = rtt.SecureReveal(BiasAdd)
r_relu1 = rtt.SecureReveal(Relu1)
r_relu0 = rtt.SecureReveal(Relu0)

r_assign_sub = rtt.SecureReveal(tf.assign_sub(Y, X))

init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    print("add zk-reveal: ", sess.run(r_add))
    print("sub zk-reveal: ", sess.run(r_sub))
    print("mul zk-reveal: ", sess.run(r_mul))
    print("matmul zk-reveal: ", sess.run(r_matmul))
    print("bias_add zk-reveal: ", sess.run(r_bias_add))
    print("relu(expect-0) zk-reveal: ", sess.run(r_relu0))
    print("relu(expect-1) zk-reveal: ", sess.run(r_relu1))
    print("assign_sub(expect-1) zk-reveal: ", sess.run(r_assign_sub))
    
    print("ending.")
