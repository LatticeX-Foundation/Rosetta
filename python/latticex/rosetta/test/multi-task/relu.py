import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Relu_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.nn.relu,
                        [0., 1., -1, 1.23638, -0.886, 4., -4., 12.0, -12., 100., -100.95], 
                        [0., 1., 0., 1.23638, 0., 4., 0., 12., 0, 100., 0.])



def Relu_2(task_id):
    return unary_op_test("Helix", task_id, tf.nn.relu,
                        [0., 1., -1, 1.23638, -0.886, 4., -4., 12.0, -12., 100., -100.95], 
                        [0., 1., 0., 1.23638, 0., 4., 0., 12., 0, 100., 0.])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Relu_1, Relu_1])
mt_unit_test_fw([Relu_2, Relu_2])
mt_unit_test_fw([Relu_1, Relu_2])
mt_unit_test_fw([Relu_1, Relu_2, Relu_1, Relu_2])
mt_unit_test_fw([Relu_1, Relu_1, Relu_1, Relu_2, Relu_2, Relu_2])


