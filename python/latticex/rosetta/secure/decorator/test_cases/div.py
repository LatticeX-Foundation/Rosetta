#!/usr/bin/python
from internal.test_cases import binary_op_test
import latticex.rosetta as cb
import tensorflow as tf

binary_op_test(tf.div, "tf.div")
# binary_op_test(cb.SecureDiv, "cb.SecureDiv")
