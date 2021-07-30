import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



def Sigmoid_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.sigmoid,
                        [0., 1., -1, 1.23638, -0.886, 4., -4., 12.0, -12., 100., -100.95], 
                        [0.5, 0.731, 0.269, 0.775, 0.292, 0.982, 0.018, 1., 0, 1., 0])


def Sigmoid_2(task_id):
    return unary_op_test("Helix", task_id, tf.sigmoid,
                        [0., 1., -1, 1.23638, -0.886, 4., -4., 12.0, -12., 100., -100.95], 
                        [0.5, 0.731, 0.269, 0.775, 0.292, 0.982, 0.018, 1., 0, 1., 0])


# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Sigmoid_1, Sigmoid_1])
mt_unit_test_fw([Sigmoid_2, Sigmoid_2])
mt_unit_test_fw([Sigmoid_1, Sigmoid_2])
mt_unit_test_fw([Sigmoid_1, Sigmoid_2, Sigmoid_1, Sigmoid_2])
mt_unit_test_fw([Sigmoid_1, Sigmoid_1, Sigmoid_1, Sigmoid_2, Sigmoid_2, Sigmoid_2])


