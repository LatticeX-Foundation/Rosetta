#!/usr/bin/python

import latticex.rosetta as rtt

import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)


protocol="SecureNN"
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
else:
    print("***** test_cases use default helix protocol ")
rtt.activate(protocol)

xa = tf.Variable(1.5)
print("xa:\n", xa)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)


print("=========================== mpc op pow const 1")
xc = tf.pow(xa, 3)
reveal_xc = rtt.SecureReveal(xc, receive_party=1)
#xcc = sess.run(xc)
reveal_xcc = sess.run(reveal_xc)
print("reveal_xcc: {}".format(reveal_xcc))
print("=========================== mpc op pow const 2")
#print(xcc)

#print("=========================== mpc op pow const 11")
xc = rtt.SecurePow(xa, "3", name='okpow')
reveal_xcc = sess.run(rtt.SecureReveal(xc))
print("reveal string xcc: ", reveal_xcc)
print("=========================== mpc op pow const 21")
#print(xcc)
###########
rtt.deactivate()
