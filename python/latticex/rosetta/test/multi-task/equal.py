import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test



def Equal_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.equal, 
                        [1.1, 2.2, -1.1, 0.], 
                        [1.1, -2.2, -1.2, 0.], 
                        [1., 0., 0., 1.])


def Equal_2(task_id):
    return bin_op_test("Helix", task_id, tf.equal, 
                        [-1.1, 2.2], 
                        2.2,
                        [0., 1.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Equal_1, Equal_1])
mt_unit_test_fw([Equal_2, Equal_2])
mt_unit_test_fw([Equal_1, Equal_2])
mt_unit_test_fw([Equal_1, Equal_2, Equal_1, Equal_2])
mt_unit_test_fw([Equal_1, Equal_1, Equal_1, Equal_2, Equal_2, Equal_2])
