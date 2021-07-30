import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def Greater_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.greater, 
                        [1.1, 2.2, -1.1, 0.1], 
                        [1.2, -2.2, -1.0, 0.], 
                        [0., 1., 0., 1.])


def Greater_2(task_id):
    return bin_op_test("Helix", task_id, tf.greater, 
                        [-1.1, 2.3, 2.1], 
                        2.2, 
                        [0., 1., 0.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Greater_1, Greater_1])
mt_unit_test_fw([Greater_2, Greater_2])
mt_unit_test_fw([Greater_1, Greater_2])
mt_unit_test_fw([Greater_1, Greater_2, Greater_1, Greater_2])
mt_unit_test_fw([Greater_1, Greater_1, Greater_1, Greater_2, Greater_2, Greater_2])
