
#!/usr/bin/python

import latticex.rosetta as cb

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


xa = tf.Variable(
    [
        [1.892, 2, 5],
        [-2.3, 4.43, 3],
        [.0091, .3, 1.2]
    ]
)

print("xa:\n", xa)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

###########
# print("=========================== tf op Argmax 1")
# xc = tf.Argmax(xa, xb)
# xcc = sess.run(xc)
# print("=========================== tf op Argmax 2")
# print(xcc)

print("=========================== mpc op Argmax 1")
xc = cb.SecureReveal(cb.SecureArgMax(xa, axis=-1))
xcc = sess.run(xc)
print("=========================== mpc op Argmax 2")
print(xcc)

print("expect: ", [2,1,2])

###########
cb.deactivate()