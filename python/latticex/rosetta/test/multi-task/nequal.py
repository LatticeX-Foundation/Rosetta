import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def Nequal_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.not_equal, 
                        [1.1, 2.2, -1.1, 0.], 
                        [1.1, -2.2, -1.2, 0.], 
                         [0., 1., 1., 0.])


def Nequal_2(task_id):
    return bin_op_test("Helix", task_id, tf.not_equal, 
                        [-1.1, 2.2], 
                        2.2, 
                        [1., 0.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Nequal_1, Nequal_1])
mt_unit_test_fw([Nequal_2, Nequal_2])
mt_unit_test_fw([Nequal_1, Nequal_2])
mt_unit_test_fw([Nequal_1, Nequal_2, Nequal_1, Nequal_2])
mt_unit_test_fw([Nequal_1, Nequal_1, Nequal_1, Nequal_2, Nequal_2, Nequal_2])
