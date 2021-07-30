import math
import time
import numpy as np
import pandas as pd
import latticex.rosetta as rtt
import tensorflow as tf


start_time = time.time()

rtt.activate("SecureNN")
mpc_player_id = rtt.py_protocol_handler.get_party_id()

# ########
a = [1.2]*10
a_ = tf.Variable(rtt.private_input(0, a))
with tf.compat.v1.Session() as sess:
    sess.run(tf.compat.v1.global_variables_initializer())
    aa = sess.run(a_)

    print('bb:', aa)
    print('type(aa):', type(aa))

b = [0.2]*10
bb = rtt.private_input(0, b)
print('bb:', bb)
print('type(bb):', type(bb))

# if (isinstance(bb, np.ndarray)):
#     print("bb is np.ndarray type")
# else:
#     print("------bb is not np.ndarray type-------")



p_bb = tf.Variable(bb)  # ok
p_aa = tf.Variable(aa) # not ok


w = tf.Variable(1.32)
# waa = w*aa  # ok
# aaw = aa*w  # not ok

try:
    with tf.compat.v1.Session() as sess:
        sess.run(tf.compat.v1.global_variables_initializer())
        sess.run(p_aa)
        # sess.run(waa)
        #sess.run(aaw)
        print("Pass")
except:
        print("Fail")


rtt.deactivate()

Writer = tf.summary.FileWriter("log/di", tf.get_default_graph())
Writer.close()
