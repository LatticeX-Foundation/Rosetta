import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, reduce_op_test



def ReduceMin_1(task_id):
    return reduce_op_test("SecureNN", task_id, tf.reduce_min,
                        [[1., 2., 3.], [4., 5., 6.]], 
                        [1.])



def ReduceMin_2(task_id):
    return reduce_op_test("Helix", task_id, tf.reduce_min,
                        [[-1., -2., -3.], [-4., -5., -6.]], 
                        [-6.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([ReduceMin_1, ReduceMin_1])
mt_unit_test_fw([ReduceMin_2, ReduceMin_2])
mt_unit_test_fw([ReduceMin_1, ReduceMin_2])
mt_unit_test_fw([ReduceMin_1, ReduceMin_2, ReduceMin_1, ReduceMin_2])
mt_unit_test_fw([ReduceMin_1, ReduceMin_1, ReduceMin_1, ReduceMin_2, ReduceMin_2, ReduceMin_2])

