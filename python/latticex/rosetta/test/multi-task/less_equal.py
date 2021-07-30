import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def LessEq_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.less_equal, 
                        [1.1, 2.2, -1.1, 0.1], 
                        [1.1, -2.2, -1.0, 0.], 
                        [1., 0., 1., 0.])


def LessEq_2(task_id):
    return bin_op_test("Helix", task_id, tf.less_equal, 
                        [-1.1, 2.3, 2.1], 
                        2.2, 
                        [1., 0., 1.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([LessEq_1, LessEq_1])
mt_unit_test_fw([LessEq_2, LessEq_2])
mt_unit_test_fw([LessEq_1, LessEq_2])
mt_unit_test_fw([LessEq_1, LessEq_2, LessEq_1, LessEq_2])
mt_unit_test_fw([LessEq_1, LessEq_1, LessEq_1, LessEq_2, LessEq_2, LessEq_2])
