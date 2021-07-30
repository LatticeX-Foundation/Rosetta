import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Neg_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.negative,
                        [1.1], [-1.1])



def Neg_2(task_id):
    return unary_op_test("Helix", task_id, tf.negative,
                        [-1.1, 3.3], [1.1, -3.3])


# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Neg_1, Neg_1])
mt_unit_test_fw([Neg_2, Neg_2])
mt_unit_test_fw([Neg_1, Neg_2])
mt_unit_test_fw([Neg_1, Neg_2, Neg_1, Neg_2])
mt_unit_test_fw([Neg_1, Neg_1, Neg_1, Neg_2, Neg_2, Neg_2])

