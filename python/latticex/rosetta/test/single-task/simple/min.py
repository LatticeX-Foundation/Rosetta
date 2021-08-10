#!/usr/bin/python

from internal.test_cases import reduce_op_test
import latticex.rosetta as cb
import tensorflow as tf


reduce_op_test(tf.reduce_min, "tf.reduce_min axis=None", axis=None)
reduce_op_test(cb.SecureMin, "cb.SecureMin axis=None", axis=None)

reduce_op_test(tf.reduce_min, "tf.reduce_min axis=0", axis=0)
reduce_op_test(cb.SecureMin, "cb.SecureMin axis=0", axis=0)

reduce_op_test(tf.reduce_min, "tf.reduce_min axis=1", axis=1)
reduce_op_test(cb.SecureMin, "cb.SecureMin axis=1", axis=1)
cb.deactivate()