import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def LogicOr_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.logical_or, 
                        [1, 1, 0, 0], 
                        [0, 1, 0, 1], 
                        [1., 1., 0., 1.])



def LogicOr_2(task_id):
    return bin_op_test("Helix", task_id, tf.logical_or, 
                        [0, 1, 0], 
                        1, 
                        [1., 1., 1.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([LogicOr_1, LogicOr_1])
mt_unit_test_fw([LogicOr_2, LogicOr_2])
mt_unit_test_fw([LogicOr_1, LogicOr_2])
mt_unit_test_fw([LogicOr_1, LogicOr_2, LogicOr_1, LogicOr_2])
mt_unit_test_fw([LogicOr_1, LogicOr_1, LogicOr_1, LogicOr_2, LogicOr_2, LogicOr_2])
