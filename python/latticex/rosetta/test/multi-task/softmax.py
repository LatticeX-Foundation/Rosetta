import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Softmax_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.softmax,
                        [9.6, 75.2, -81.3, 8.04, 2.27, -70.95, -130.95, -123.51, 137.98, 89.54], 
                        [0., 0., 0., 0., 0., 0., 0., 0, 1., 0.])



def Softmax_2(task_id):
    return unary_op_test("Helix", task_id, tf.softmax,
                        [9.6, 75.2, -81.3, 8.04, 2.27, -70.95, -130.95, -123.51, 137.98, 89.54], 
                        [0., 0., 0., 0., 0., 0., 0., 0, 1., 0.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Softmax_1, Softmax_1])
mt_unit_test_fw([Softmax_2, Softmax_2])
mt_unit_test_fw([Softmax_1, Softmax_2])
mt_unit_test_fw([Softmax_1, Softmax_2, Softmax_1, Softmax_2])
mt_unit_test_fw([Softmax_1, Softmax_1, Softmax_1, Softmax_2, Softmax_2, Softmax_2])


