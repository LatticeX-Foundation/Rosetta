import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Argmax_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.argmax,
                        [[1., 2., 3.], [2., 3., 4.], [5., 4., 3.], [8., 7., 2.]], 
                        [2, 2, 0, 0])


def Argmax_2(task_id):
    return unary_op_test("Helix", task_id, tf.argmax,
                        [[-1., -2., -3.], [-2., -3., -4.], [-5., -4., -3.], [-8., -7., -2.]], 
                        [0, 0, 2, 2])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Argmax_1, Argmax_1])
mt_unit_test_fw([Argmax_2, Argmax_2])
mt_unit_test_fw([Argmax_1, Argmax_2])
mt_unit_test_fw([Argmax_1, Argmax_2, Argmax_1, Argmax_2])
mt_unit_test_fw([Argmax_1, Argmax_1, Argmax_1, Argmax_2, Argmax_2, Argmax_2])

