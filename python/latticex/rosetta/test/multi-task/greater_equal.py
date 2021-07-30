import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def GreaterEqu_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.greater_equal, 
                        [1.1, 2.2, -1.1, 0.1], 
                        [1.2, -2.2, -1.0, 0.], 
                        [0., 1., 0., 1.])



def GreaterEqu_2(task_id):
    return bin_op_test("Helix", task_id, tf.greater_equal, 
                        [-1.1, 2.3, 2.2], 
                        2.2, 
                        [0., 1., 1.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([GreaterEqu_1, GreaterEqu_1])
mt_unit_test_fw([GreaterEqu_2, GreaterEqu_2])
mt_unit_test_fw([GreaterEqu_1, GreaterEqu_2])
mt_unit_test_fw([GreaterEqu_1, GreaterEqu_2, GreaterEqu_1, GreaterEqu_2])
mt_unit_test_fw([GreaterEqu_1, GreaterEqu_1, GreaterEqu_1, GreaterEqu_2, GreaterEqu_2, GreaterEqu_2])
