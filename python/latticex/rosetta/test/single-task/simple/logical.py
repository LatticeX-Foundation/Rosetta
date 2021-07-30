#!/usr/bin/python

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

# rtt.activate("SecureNN")
rtt.activate("Helix")

#
# the following case from c++ tests
# (see protocol/mpc/tests/op_logical.cpp)
#
X1 = [1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1]
X2 = [0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0]
# X= [1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1] # X1>X2
Y1 = [1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1]
Y2 = [0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0]
# Y= [1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1] # Y1>Y2
#AND [1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1]
#XOR [0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0]
# OR [1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1]
#OTx [0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0]

x1 = tf.Variable(X1)
x2 = tf.Variable(X2)
y1 = tf.Variable(Y1)
y2 = tf.Variable(Y2)

X = tf.greater(x1, x2)
Y = tf.greater(y1, y2)

z_and = rtt.SecureLogicalAnd(X, Y)
z_or = rtt.SecureLogicalOr(X, Y)
z_xor = rtt.SecureLogicalXor(X, Y)
z_not = rtt.SecureLogicalNot(X)

r_and = rtt.SecureReveal(z_and)
r_or = rtt.SecureReveal(z_or)
r_xor = rtt.SecureReveal(z_xor)
r_not = rtt.SecureReveal(z_not)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
res_and = sess.run(r_and)
res_or = sess.run(r_or)
res_xor = sess.run(r_xor)
res_not = sess.run(r_not)

#
print("res_and: ", res_and)
print("res_or : ", res_or)
print("res_xor: ", res_xor)
print("res_not: ", res_not)

rtt.deactivate()
