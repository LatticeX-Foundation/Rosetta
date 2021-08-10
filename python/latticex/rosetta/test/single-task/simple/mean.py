#!/usr/bin/python

from internal.test_cases import reduce_op_test
import latticex.rosetta as cb
import tensorflow as tf

reduce_op_test(tf.reduce_mean, "tf.reduce_mean axis=None", axis=None)
reduce_op_test(cb.SecureMean, "cb.SecureMean axis=None", axis=None)

reduce_op_test(tf.reduce_mean, "tf.reduce_mean axis=0", axis=0)
reduce_op_test(cb.SecureMean, "cb.SecureMean axis=0", axis=0)

reduce_op_test(tf.reduce_mean, "tf.reduce_mean axis=1", axis=1)
reduce_op_test(cb.SecureMean, "cb.SecureMean axis=1", axis=1)
cb.deactivate()