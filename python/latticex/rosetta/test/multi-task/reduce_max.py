import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, reduce_op_test



def ReduceMax_1(task_id):
    return reduce_op_test("SecureNN", task_id, tf.reduce_max,
                        [[1., 2., 3.], [4., 5., 6.]], 
                        [6.])


def ReduceMax_2(task_id):
    return reduce_op_test("Helix", task_id, tf.reduce_max,
                        [[-1., -2., -3.], [-4., -5., -6.]], 
                        [-1.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([ReduceMax_1, ReduceMax_1])
mt_unit_test_fw([ReduceMax_2, ReduceMax_2])
mt_unit_test_fw([ReduceMax_1, ReduceMax_2])
mt_unit_test_fw([ReduceMax_1, ReduceMax_2, ReduceMax_1, ReduceMax_2])
mt_unit_test_fw([ReduceMax_1, ReduceMax_1, ReduceMax_1, ReduceMax_2, ReduceMax_2, ReduceMax_2])

