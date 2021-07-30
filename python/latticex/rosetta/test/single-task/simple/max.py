#!/usr/bin/python
from internal.test_cases import reduce_op_test
import numpy as np
import tensorflow as tf
MIN_MAX_EXPECT = np.array([])

AXIS_NONE = reduce_op_test(tf.reduce_max, "tf.reduce_max axis=None", axis=None)
MIN_MAX_EXPECT = np.append(MIN_MAX_EXPECT, AXIS_NONE.flatten())
AXIS_ZERO = reduce_op_test(tf.reduce_max, "tf.reduce_max axis=0", axis=0)
MIN_MAX_EXPECT = np.append(MIN_MAX_EXPECT, AXIS_ZERO.flatten())
AXIS_ONE = reduce_op_test(tf.reduce_max, "tf.reduce_max axis=1", axis=1)
MIN_MAX_EXPECT = np.append(MIN_MAX_EXPECT, AXIS_ONE.flatten())

AXIS_NONE = reduce_op_test(tf.reduce_min, "tf.reduce_min axis=None", axis=None)
MIN_MAX_EXPECT = np.append(MIN_MAX_EXPECT, AXIS_NONE.flatten())
AXIS_ZERO = reduce_op_test(tf.reduce_min, "tf.reduce_min axis=0", axis=0)
MIN_MAX_EXPECT = np.append(MIN_MAX_EXPECT, AXIS_ZERO.flatten())
AXIS_ONE = reduce_op_test(tf.reduce_min, "tf.reduce_min axis=1", axis=1)
MIN_MAX_EXPECT = np.append(MIN_MAX_EXPECT, AXIS_ONE.flatten())


import latticex.rosetta as rtt
#rtt.activate("SecureNN")
#rtt.py_protocol_handler.set_loglevel(0)
rtt.activate("Helix")
MPC_RES = np.array([])
AXIS_NONE = reduce_op_test(rtt.SecureMax, "rtt.SecureMax axis=None", axis=None)
MPC_RES = np.append(MPC_RES, AXIS_NONE.flatten())
AXIS_ZERO = reduce_op_test(rtt.SecureMax, "rtt.SecureMax axis=0", axis=0)
MPC_RES = np.append(MPC_RES, AXIS_ZERO.flatten())
AXIS_ONE = reduce_op_test(rtt.SecureMax, "rtt.SecureMax axis=1", axis=1)
MPC_RES = np.append(MPC_RES, AXIS_ONE.flatten())

AXIS_NONE = reduce_op_test(rtt.SecureMin, "rtt.SecureMin axis=None", axis=None)
MPC_RES = np.append(MPC_RES, AXIS_NONE.flatten())
AXIS_ZERO = reduce_op_test(rtt.SecureMin, "rtt.SecureMin axis=0", axis=0)
MPC_RES = np.append(MPC_RES, AXIS_ZERO.flatten())
AXIS_ONE = reduce_op_test(rtt.SecureMin, "rtt.SecureMin axis=1", axis=1)
MPC_RES = np.append(MPC_RES, AXIS_ONE.flatten())

print("expect:\n", MIN_MAX_EXPECT)
print("real:\n", MPC_RES)

print("*" * 30 + "CHECKING" + "*" * 30)
try:
    np.testing.assert_allclose(MIN_MAX_EXPECT, MPC_RES, rtol=0, atol=1.0/(2**10))
    print("SUCCESS!!!! PASSED!!!")
except Exception as e:
    print("FAIL!!!")
    print("context:", e)
print("*" * 69)
