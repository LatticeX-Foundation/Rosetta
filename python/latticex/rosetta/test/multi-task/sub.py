import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test
import latticex.rosetta as rtt



def Sub_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.subtract, 
                        [-2.2], [1.1], [-3.3])


def Sub_2(task_id):
    return bin_op_test("Helix", task_id, tf.subtract, 
                        [-1.1, 3.3], 2.2, [-3.3, 1.1])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Sub_1, Sub_1])
mt_unit_test_fw([Sub_2, Sub_2])
mt_unit_test_fw([Sub_1, Sub_2])
mt_unit_test_fw([Sub_1, Sub_2, Sub_1, Sub_2])
mt_unit_test_fw([Sub_1, Sub_1, Sub_1, Sub_2, Sub_2, Sub_2])

