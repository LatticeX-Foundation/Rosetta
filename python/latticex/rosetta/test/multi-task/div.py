import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test
import latticex.rosetta as rtt



def Div_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.truediv, 
                        [1.1], [2.2], [0.5])


def Div_2(task_id):
    return bin_op_test("Helix", task_id, tf.truediv, 
                        [-1.1, 3.3], 2.2, [-0.5, 1.5])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Div_1, Div_1])
mt_unit_test_fw([Div_2, Div_2])
mt_unit_test_fw([Div_1, Div_2])
mt_unit_test_fw([Div_1, Div_2, Div_1, Div_2])
mt_unit_test_fw([Div_1, Div_1, Div_1, Div_2, Div_2, Div_2])

