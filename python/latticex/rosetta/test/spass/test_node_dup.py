import math
import time
import numpy as np
import pandas as pd
import latticex.rosetta as rtt
import tensorflow as tf


import latticex.rosetta as cb

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

rtt.activate("SecureNN")
xa = tf.Variable(
    [
        [1.892, 2],
        [-2.3, 4.43],
        [.0091, .3]
    ]
)
xb = tf.Variable(
    [
        [2.892, 2],
        [-2.3, 4.43],
        [.0091, -0.3]
    ]
)

print("xa:\n", xa)
print("xb:\n", xb)

#
try:
    init = tf.compat.v1.global_variables_initializer()
    sess = tf.compat.v1.Session()
    sess.run(init)

    ###########
    print("=========================== tf op greater 1")
    xc = tf.greater(xa, xb)
    xcc = sess.run(xc)
    print("=========================== tf op greater 2")
    print(xcc)

    print("=========================== mpc op greater 1")
    xc = cb.SecureGreater(xa, xb)
    xcc = sess.run(xc)
    xcc = sess.run(xc)
    print("=========================== mpc op greater 2")
    print(xcc)

    print("Pass")
except Exception as e:
    print(str(e))
    print("Fail")


rtt.deactivate()
Writer = tf.summary.FileWriter("log/node_dup", tf.get_default_graph())
Writer.close()
