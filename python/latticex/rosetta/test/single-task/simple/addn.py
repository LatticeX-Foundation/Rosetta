#!/usr/bin/python

import latticex.rosetta as rst

import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

# rst.set_backend_loglevel(1)
# rst.backend_log_to_stdout(True)


rst.activate("SecureNN")
rst.set_backend_loglevel(1)
rst.backend_log_to_stdout(True)
# rst.set_backend_logfile("log/addn.{}".format(rst.get_party_id()))
xa = tf.Variable(rst.private_input(0,
    [
        [-102.12345678],
        [-0.12345678],
        [0.234567890],
        [0.34567890]
    ])
)
# xa = tf.Variable(0,
#     [
#         [-102.12345678],
#         [-0.12345678],
#         [0.234567890],
#         [0.34567890]
#     ]
# )
# xb = tf.Variable([[1],[2],[2],[1]])

# print("xa:\n", xa)

# #
init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)
# print("Input Xa:", sess.run(xa*2))
# ###########
# print("=========================== tf op add_n 1")
# xc = tf.add_n([1,2])
# xcc = sess.run(xc)
# print("=========================== tf op add_n 2")
# print(xcc)

print("=========================== mpc op SecureAddN 1")
xc = rst.SecureAddN([xa, xa])
xcc = sess.run(xc)
print("=========================== mpc op SecureAddN 2")

print(xcc)
###########

writer = tf.summary.FileWriter("log/graph", tf.get_default_graph())
writer.close()