import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def Less_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.less, 
                        [1.1, 2.2, -1.1, 0.], 
                        [1.2, -2.2, -1.0, 0.], 
                        [1., 0., 1., 0.])


def Less_2(task_id):
    return bin_op_test("Helix", task_id, tf.less, 
                        [-1.1, 2.3, 2.1], 
                        2.2, 
                        [1., 0., 1.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Less_1, Less_1])
mt_unit_test_fw([Less_2, Less_2])
mt_unit_test_fw([Less_1, Less_2])
mt_unit_test_fw([Less_1, Less_2, Less_1, Less_2])
mt_unit_test_fw([Less_1, Less_1, Less_1, Less_2, Less_2, Less_2])
