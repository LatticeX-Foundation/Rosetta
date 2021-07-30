import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, reduce_op_test



def ReduceMean_1(task_id):
    return reduce_op_test("SecureNN", task_id, tf.reduce_mean,
                        [[1., 2., 3.], [4., 5., 6.]], 
                        [3.5])



def ReduceMean_2(task_id):
    return reduce_op_test("Helix", task_id, tf.reduce_mean,
                        [[-1., -2., -3.], [-4., -5., -6.]], 
                        [-3.5])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([ReduceMean_1, ReduceMean_1])
mt_unit_test_fw([ReduceMean_2, ReduceMean_2])
mt_unit_test_fw([ReduceMean_1, ReduceMean_2])
mt_unit_test_fw([ReduceMean_1, ReduceMean_2, ReduceMean_1, ReduceMean_2])
mt_unit_test_fw([ReduceMean_1, ReduceMean_1, ReduceMean_1, ReduceMean_2, ReduceMean_2, ReduceMean_2])

