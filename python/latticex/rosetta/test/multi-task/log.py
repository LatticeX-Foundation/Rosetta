import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Log_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.log,
                        [[1.56, 2], [3.3, 4.43], [10.0, 1.85]], 
                        [[0.445, 0.693], [1.194, 1.488], [2.303, 0.615]])


def Log_2(task_id):
    return unary_op_test("Helix", task_id, tf.log,
                        [[8.2, 0.2], [0.425, 1], [5.6, 0.1]], 
                        [[2.104, -1.609], [-0.856, 0], [1.723, -2.303]])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Log_1, Log_1])
mt_unit_test_fw([Log_2, Log_2])
mt_unit_test_fw([Log_1, Log_2])
mt_unit_test_fw([Log_1, Log_2, Log_1, Log_2])
mt_unit_test_fw([Log_1, Log_1, Log_1, Log_2, Log_2, Log_2])


