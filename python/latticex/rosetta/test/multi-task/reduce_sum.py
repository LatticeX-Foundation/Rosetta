import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, reduce_op_test



def ReduceSum_1(task_id):
    return reduce_op_test("SecureNN", task_id, tf.reduce_sum,
                        [[1., 2., 3.], [4., 5., 6.]], 
                        [21.])



def ReduceSum_2(task_id):
    return reduce_op_test("Helix", task_id, tf.reduce_sum,
                        [[-1., -2., -3.], [-4., -5., -6.]], 
                        [-21.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([ReduceSum_1, ReduceSum_1])
mt_unit_test_fw([ReduceSum_2, ReduceSum_2])
mt_unit_test_fw([ReduceSum_1, ReduceSum_2])
mt_unit_test_fw([ReduceSum_1, ReduceSum_2, ReduceSum_1, ReduceSum_2])
mt_unit_test_fw([ReduceSum_1, ReduceSum_1, ReduceSum_1, ReduceSum_2, ReduceSum_2, ReduceSum_2])

