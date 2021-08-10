#!/usr/bin/python

from internal.test_cases import binary_op_test
import latticex.rosetta as cb
import tensorflow as tf

binary_op_test(tf.truediv, "tf.truediv")
binary_op_test(cb.SecureTruediv, "cb.SecureTruediv")
cb.deactivate()