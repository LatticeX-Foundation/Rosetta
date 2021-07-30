import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Log1p_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.log1p,
                        [[1.56, 2], [3.3, 4.43], [9.5, 1.85]], 
                        [[0.94, 1.099], [1.459, 1.692], [2.35, 1.047]])


def Log1p_2(task_id):
    return unary_op_test("Helix", task_id, tf.log1p,
                        [[6.6, 2.1], [4.235, 1], [5.6, 0.1]], 
                        [[2.028, 1.131], [1.655, 0.693], [1.8, 0.095]])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Log1p_1, Log1p_1])
mt_unit_test_fw([Log1p_2, Log1p_2])
mt_unit_test_fw([Log1p_1, Log1p_2])
mt_unit_test_fw([Log1p_1, Log1p_2, Log1p_1, Log1p_2])
mt_unit_test_fw([Log1p_1, Log1p_1, Log1p_1, Log1p_2, Log1p_2, Log1p_2])



