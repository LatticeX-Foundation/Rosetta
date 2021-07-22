import os
print("PWD:", os.getcwd())

#import unittest

import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf

# our package
import latticex.rosetta as cb
#cb.activate("Helix")
cb.activate("SecureNN")

party_ID = cb.get_party_id()
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

init_op = tf.compat.v1.global_variables_initializer()

print("========Local init input values================")
with tf.compat.v1.Session() as pre_sess:
    pre_sess.run(init_op)
    print("xa:\n", xa.eval(pre_sess))
    print("xb:\n", xb.eval(pre_sess))

SAVED_FILE_PREFIX = './log/MPC_SAVE' + "_P" + str(party_ID)
SAVED_PATH = SAVED_FILE_PREFIX

print("========SecureSaveV2 test BEGIN================")
# S1: save the shared value to file with its real value. 
with tf.compat.v1.Session() as mpc_save_sess:
    mpc_save_sess.run(init_op)
    # assign_op1 = tf.assign(xa, xa * 2.0)
    # assign_op2 = tf.assign(xb, xb - 1.0)
    # mpc_save_sess.run([assign_op1, assign_op2])
    # (na, nb) = mpc_save_sess.run([xa, xb])
    # print([na, nb])
    mpc_save_v2_op = cb.SecureSaveV2(SAVED_FILE_PREFIX, # the filename_tensor
                        ["v1","v2"], # the tensor_names
                        ['', ''], # the tensor_sclice
                        [xa, xb]) # real tensors
    mpc_save_sess.run(mpc_save_v2_op)

print("========SecureSaveV2 restoring CIPHER result================")
# to validate that the recovered value is true. 
# Note that the following should be runable only if you set the SAVE_MODE as '0'!
with tf.compat.v1.Session() as sess3:
    # NO NEED TO INIT THE VARIABLE. WE WILL RESTORE IT!
    # pay attention to that the default graph is defined globally, 
    # so the sess2 and the sess3 share the same graph structure! 
    # 
    #sess3.run(init_op)
    #[attention!]: the last one dir must be  a new one!
    my_sv = tf.compat.v1.train.Saver({"v1": xa, "v2": xb})
    my_sv.restore(sess3, SAVED_PATH)
    a_out, b_out = sess3.run([xa, xb])
    print('CIPHER model is restored from {}'.format(SAVED_PATH))
    print('restored xa=', a_out)
    print('restored xb=', b_out)

print("========SecureSaveV2 test END================")

