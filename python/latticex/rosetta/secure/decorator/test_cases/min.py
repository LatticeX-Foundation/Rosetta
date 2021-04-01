#!/usr/bin/python

from internal.test_cases import reduce_op_test
import latticex.rosetta as rtt
import tensorflow as tf


reduce_op_test(tf.reduce_min, "tf.reduce_min axis=None", axis=None)
reduce_op_test(rtt.SecureMin, "rtt.SecureMin axis=None", axis=None)

reduce_op_test(tf.reduce_min, "tf.reduce_min axis=0", axis=0)
reduce_op_test(rtt.SecureMin, "rtt.SecureMin axis=0", axis=0)

reduce_op_test(tf.reduce_min, "tf.reduce_min axis=1", axis=1)
reduce_op_test(rtt.SecureMin, "rtt.SecureMin axis=1", axis=1)
