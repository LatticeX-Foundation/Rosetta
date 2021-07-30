import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Square_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.square,
                        [[1.56, 2], [3.3, 4.43], [100.0, 1.85]], 
                        [[2.4336, 4.0], [10.89, 19.625], [10000.0, 3.423]])



def Square_2(task_id):
    return unary_op_test("Helix", task_id, tf.square,
                        [[-11.26, -200], [-3.3, -4.43], [-100.0, -1.85]], 
                        [[126.788, 40000], [10.89, 19.625], [10000, 3.423]])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Square_1, Square_1])
mt_unit_test_fw([Square_2, Square_2])
mt_unit_test_fw([Square_1, Square_2])
mt_unit_test_fw([Square_1, Square_2, Square_1, Square_2])
mt_unit_test_fw([Square_1, Square_1, Square_1, Square_2, Square_2, Square_2])
