import os
print("my ProcessID:", os.getpid())
print("PWD:", os.getcwd())

#import unittest

import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf

# our package
import latticex.rosetta as cb

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
#xa = tf.Variable(10, dtype = tf.int32)
#xb = tf.Variable(20, dtype = tf.int32)

init_op = tf.compat.v1.global_variables_initializer()

print("========Local init input values================")
with tf.compat.v1.Session() as pre_sess:
    pre_sess.run(init_op)
    print("xa:\n", xa.eval(pre_sess))
    print("xb:\n", xb.eval(pre_sess))

# Note: the reason to append pid is to aviod file name dulication 
#       when multi-party run in the same host.
SAVED_FILE_PREFIX = './log/MPC_SAVE' + "_" + str(os.getpid())
SAVED_PATH = SAVED_FILE_PREFIX

print("========SecureSaveV2 test BEGIN================")

my_saver = tf.train.Saver({"v1": xa, "v2": xb})
#my_sv = tf.compat.v1.train.Saver({"v1": xa, "v2": xb})
# S1: save the shared value to file with its real value. 
with tf.compat.v1.Session() as mpc_save_sess:
    mpc_save_sess.run(init_op)
    #mpc_save_sess.run(mpc_save_v2_op)
    SAVED_PATH = my_saver.save(mpc_save_sess, SAVED_FILE_PREFIX)
    print('Model is saved in {}'.format(SAVED_PATH))

# S2: to validate that the recovered value is true. 
with tf.compat.v1.Session() as mpc_check_result_sess:
    # NO NEED TO INIT THE VARIABLE. WE WILL RESTORE IT!
    # pay attention to that the default graph is defined globally, 
    # so the mpc_check_result_sess and the mpc_save_sess should
    # share the same graph structure! 
    mpc_check_result_sess.run(init_op)
    my_sv = tf.train.Saver({"v1": xa, "v2": xb})
    my_sv.restore(mpc_check_result_sess, SAVED_PATH)
    a_out, b_out = mpc_check_result_sess.run([xa, xb])
    print('model is restored from {}'.format(SAVED_PATH))
    print('restored xa=', a_out)
    print('restored xb=', b_out)
    precision_target = np.full(num_a.shape, PRECISION, np.float_)
    # TODO: different party ID with different result
    if ((abs(np.double(a_out) - 2 * num_a) < precision_target).all() and \
        (abs(np.double(b_out) - 2* num_b) < precision_target).all()):
        print("num_a and num_b restored correctly!")
    else:
        print("num_a or num_b restored failed!")

print("========SecureSaveV2 test END================")
cb.deactivate()