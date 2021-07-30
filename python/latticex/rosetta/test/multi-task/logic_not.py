import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def LogicNot_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.logical_not,
                        [1, 1, 0, 0], 
                        [0., 0., 1., 1.])



def LogicNot_2(task_id):
    return unary_op_test("Helix", task_id, tf.logical_not,
                        [0, 1, 0], 
                        [1., 0., 1.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([LogicNot_1, LogicNot_1])
mt_unit_test_fw([LogicNot_2, LogicNot_2])
mt_unit_test_fw([LogicNot_1, LogicNot_2])
mt_unit_test_fw([LogicNot_1, LogicNot_2, LogicNot_1, LogicNot_2])
mt_unit_test_fw([LogicNot_1, LogicNot_1, LogicNot_1, LogicNot_2, LogicNot_2, LogicNot_2])
