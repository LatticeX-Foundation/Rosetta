import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def LogicXor_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.logical_xor, 
                        [1, 1, 0, 0], 
                        [0, 1, 0, 1], 
                        [1., 0., 0., 1.])



def LogicXor_2(task_id):
    return bin_op_test("Helix", task_id, tf.logical_xor, 
                        [0, 1, 0], 
                        1, 
                        [1., 0., 1.])
                        


# ===========================
# run test cases
# ===========================
mt_unit_test_fw([LogicXor_1, LogicXor_1])
mt_unit_test_fw([LogicXor_2, LogicXor_2])
mt_unit_test_fw([LogicXor_1, LogicXor_2])
mt_unit_test_fw([LogicXor_1, LogicXor_2, LogicXor_1, LogicXor_2])
mt_unit_test_fw([LogicXor_1, LogicXor_1, LogicXor_1, LogicXor_2, LogicXor_2, LogicXor_2])
