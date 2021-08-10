#!/usr/bin/python

from internal.test_cases import binary_op_test
import latticex.rosetta as cb
import tensorflow as tf

binary_op_test(tf.add, "tf.add")
binary_op_test(cb.SecureAdd, "cb.SecureAdd")
cb.deactivate()
