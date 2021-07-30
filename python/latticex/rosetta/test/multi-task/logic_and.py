import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def LogicAnd_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.logical_and, 
                        [1, 1, 0, 0], 
                        [0, 1, 0, 1], 
                        [0., 1., 0., 0.])



def LogicAnd_2(task_id):
    return bin_op_test("Helix", task_id, tf.logical_and, 
                        [0, 1, 0], 
                        1, 
                        [0., 1., 0.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([LogicAnd_1, LogicAnd_1])
mt_unit_test_fw([LogicAnd_2, LogicAnd_2])
mt_unit_test_fw([LogicAnd_1, LogicAnd_2])
mt_unit_test_fw([LogicAnd_1, LogicAnd_2, LogicAnd_1, LogicAnd_2])
mt_unit_test_fw([LogicAnd_1, LogicAnd_1, LogicAnd_1, LogicAnd_2, LogicAnd_2, LogicAnd_2])
