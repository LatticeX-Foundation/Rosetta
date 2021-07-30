import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_rh_const_test



def Pow_1(task_id):
    return bin_op_rh_const_test("SecureNN", task_id, tf.pow, 
                        [1.3, -2.1, -3.8, 41.43], 2.0, 
                        [1.69, 4.41, 14.44, 1716.445])



def Pow_2(task_id):
    return bin_op_rh_const_test("Helix", task_id, tf.pow, 
                        [1.3, -2.1, 20.0, -13.1], 2.0, 
                        [1.69, 4.41, 400.0, 171.61])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Pow_1, Pow_1])
mt_unit_test_fw([Pow_2, Pow_2])
mt_unit_test_fw([Pow_1, Pow_2])
mt_unit_test_fw([Pow_1, Pow_2, Pow_1, Pow_2])
mt_unit_test_fw([Pow_1, Pow_1, Pow_1, Pow_2, Pow_2, Pow_2])

