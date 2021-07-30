import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Abs_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.abs,
                        [0., 1., -1., 65536., -65536.], 
                        [0., 1., 1., 65536., 65536.])


def Abs_2(task_id):
    return unary_op_test("Helix", task_id, tf.abs,
                        [0., 1., -1., 65536., -65536.], 
                        [0., 1., 1., 65536., 65536.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Abs_1, Abs_1])
mt_unit_test_fw([Abs_2, Abs_2])
mt_unit_test_fw([Abs_1, Abs_2])
mt_unit_test_fw([Abs_1, Abs_2, Abs_1, Abs_2])
mt_unit_test_fw([Abs_1, Abs_1, Abs_1, Abs_2, Abs_2, Abs_2])



