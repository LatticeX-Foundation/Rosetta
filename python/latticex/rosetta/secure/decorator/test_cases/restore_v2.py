#!/usr/bin/python

import latticex.rosetta as cb

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


xa = tf.Variable(
    [
        [1.892, 2],
        [3, 4.43],
        [.0091, .3]
    ]
)
xb = tf.Variable(
    [
        [5.3,  .7],
        [6, -0.7],
        [3,  -2.001]
    ]
)

print("xa:\n", xa)
print("xb:\n", xb)

#
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

###########


###########
