import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test
import latticex.rosetta as rtt



def Mul_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.multiply, 
                        [1.1], [2.2], [2.42])


def Mul_2(task_id):
    return bin_op_test("Helix", task_id, tf.multiply, 
                        [-1.1, 3.3], 2.2, [-2.42, 7.26])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Mul_1, Mul_1])
mt_unit_test_fw([Mul_2, Mul_2])
mt_unit_test_fw([Mul_1, Mul_2])
mt_unit_test_fw([Mul_1, Mul_2, Mul_1, Mul_2])
mt_unit_test_fw([Mul_1, Mul_1, Mul_1, Mul_2, Mul_2, Mul_2])
