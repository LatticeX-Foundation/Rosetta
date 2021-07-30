#!/usr/bin/env python3

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

protocol = "Helix"
rtt.activate(protocol)

patyid = str(rtt.get_party_id())
rtt.set_backend_loglevel(0)
rtt.set_backend_logfile("/tmp/wo-men-dou-shi-hao-hai-zi.log"+patyid)

print("rtt.get_protocol_name():", rtt.get_protocol_name())

X = tf.Variable([[1., 1.], [2., 1.]])
Y = tf.Variable([[1., 3.], [1., 1.]])
z = rtt.SecureMatMul(X, Y)
zr = rtt.SecureReveal(z)

init = tf.global_variables_initializer()
with tf.Session() as sess1:
    sess1.run(init)
    print('zzzzzr:', sess1.run(zr))


rtt.set_backend_loglevel(3)
rtt.set_backend_logfile("/tmp/wo-men-dou-shi-hao-hai-zi-ma.log"+patyid)

with tf.Session() as sess1:
    sess1.run(init)
    print('zzzzzr:', sess1.run(zr))


exit(0)
