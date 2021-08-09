#!/usr/bin/python

from internal.test_cases import reduce_op_test
import latticex.rosetta as cb
import tensorflow as tf


reduce_op_test(tf.reduce_sum, "tf.reduce_sum axis=None", axis=None)
reduce_op_test(cb.SecureSum, "cb.SecureSum axis=None", axis=None)

reduce_op_test(tf.reduce_sum, "tf.reduce_sum axis=0", axis=0)
reduce_op_test(cb.SecureSum, "cb.SecureSum axis=0", axis=0)

reduce_op_test(tf.reduce_sum, "tf.reduce_sum axis=1", axis=1)
reduce_op_test(cb.SecureSum, "cb.SecureSum axis=1", axis=1)
cb.deactivate()
