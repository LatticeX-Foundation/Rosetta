#!/usr/bin/python

import latticex.rosetta as cb

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


xa = np.double(1.5)
print("xa:\n", xa)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)


print("=========================== mpc op pow const 1")
xc = cb.SecurePow(xa, np.double(3))
reveal_xc = cb.SecureReveal(xc, reveal_party=0)
#xcc = sess.run(xc)
reveal_xcc = sess.run(reveal_xc)
print("reveal_xcc: {}".format(reveal_xcc))
print("=========================== mpc op pow const 2")
#print(xcc)

#print("=========================== mpc op pow const 11")
#xc = cb.SecurePow(xa, 3.3, name='okpow')
#xcc = sess.run(xc)
#print("=========================== mpc op pow const 21")
#print(xcc)
###########
