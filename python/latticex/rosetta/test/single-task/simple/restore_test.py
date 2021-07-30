import os
print("my ProcessID:", os.getpid())
print("PWD:", os.getcwd())

#import unittest

import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf

# two initial values with double type.
num_a = np.array(
    [
        [1, 2],
        [3, 4]
    ], dtype =np.float_)
num_b = np.array(
    [
        [10.1, 20.02],
        [30.003, 40.0004] 
    ],dtype= np.float_)

# the precision requirement of saving and restoring a float number
PRECISION = 1.0/1000

xa = tf.Variable(num_a, dtype = tf.double)
xb = tf.Variable(num_b, dtype = tf.double)

SAVED_FILE_PREFIX = './log/MPC_SAVE' + "_P0"
SAVED_PATH = SAVED_FILE_PREFIX

# Note that is is only for restoring the palintext saved for parties you 
# specified as SAVE_MODE in config file! 
print("========SecureSaveV2 restoring PLAIN result================")
# S2: to validate that the recovered value is true. 
with tf.compat.v1.Session() as mpc_check_result_sess:
    # NO NEED TO INIT THE VARIABLE. WE WILL RESTORE IT!
    # pay attention to that the default graph is defined globally, 
    # so the mpc_check_result_sess and the mpc_save_sess should
    # share the same graph structure! 
    # mpc_check_result_sess.run(init_op)
    my_sv = tf.compat.v1.train.Saver({"v1": xa, "v2": xb})
    my_sv.restore(mpc_check_result_sess, SAVED_PATH)
    a_out, b_out = mpc_check_result_sess.run([xa, xb])
    print('model is restored from {}'.format(SAVED_PATH))
    print('restored xa=', a_out)
    print('restored xb=', b_out)
    precision_target = np.full(num_a.shape, PRECISION, np.float_)
    if ((abs(a_out - num_a) < precision_target).all() and \
        (abs(b_out - num_b) < precision_target).all()):
        print("num_a and num_b restored correctly!")
    else:
        print("num_a or num_b restored failed!")

print("========SecureSaveV2 test END================")
