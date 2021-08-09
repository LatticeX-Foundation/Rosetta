#!/usr/bin/python

from internal.test_cases import binary_op_test
import latticex.rosetta as cb
import tensorflow as tf

binary_op_test(tf.subtract, "tf.subtract")
binary_op_test(cb.SecureSub, "cb.SecureSub")
cb.deactivate()